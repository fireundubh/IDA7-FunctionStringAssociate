// ****************************************************************************
// File: Core.cpp
// Desc: Function String Associate plug-in
//
// ****************************************************************************
#include "StdAfx.h"
#include <WaitBoxEx.h>

const unsigned int MAX_LINE_STR_COUNT = 10;
const size_t MAX_LABEL_STR = 60; // Max size of each label
const size_t MAX_COMMENT = 764;  // Max size of whole comment line

// === Function Prototypes ===
int idaapi IDAP_init();
void idaapi IDAP_term();
bool idaapi IDAP_run(size_t arg);

// === Data ===
const static char IDAP_comment[] = "Extracts strings from each function and intelligently adds them to function comments.";
const static char IDAP_help[] = "";
const static char IDAP_name[] = "Function String Associate";

// Working label element info container
#pragma pack(push, 1)
struct STRC
{
	char str[MAX_LABEL_STR];
	int refs;
};
#pragma pack(pop)

// === Function Prototypes ===
static void processFunction(func_t* f);

// === Data ===
static ALIGN(16) STRC stringArray[MAX_LINE_STR_COUNT];
static int64 commentCount = 0;

// Main dialog
static const char mainDialog[] =
{
	"BUTTON YES* Continue\n" // "Continue" instead of "okay"
	"Function String Associate\n"

	#ifdef _DEBUG
	"** DEBUG BUILD **\n"
	#endif
	"Extracts strings from each function and intelligently adds them  \nto the function comment line.\n\n"
	"Version %Aby Sirmabus\n"
	"<#Click to open site.#www.macromonkey.com:k:1:1::>\n\n"

	" \n\n\n\n\n"
};

// Initialize
int idaapi IDAP_init()
{
	// String container should be align 16
	_ASSERT((sizeof(STRC) & 7) == 0);
	return PLUGIN_OK;
}

// Un-initialize
void idaapi IDAP_term()
{
}

// Plug-in process
bool idaapi IDAP_run(size_t arg)
{
	try
	{
		char version[16];
		sprintf(version, "%u.%u", HIBYTE(MY_VERSION), LOBYTE(MY_VERSION));
		msg("\n>> Function String Associate: v: %s, built: %s, By Sirmabus\n", version, __DATE__);

		if (auto_is_ok())
		{
			refreshUI();

			const int iUIResult = ask_form(mainDialog, version);
			if (!iUIResult)
			{
				msg(" - Canceled -\n");
				return false;
			}

			WaitBox::show();

			// Iterate through all functions..            
			const size_t functionCount = get_func_qty();
			char count[32] = "";
			snprintf(count, sizeof(count), "%zu", functionCount);

			msg("Processing %s functions.\n", count);

			refreshUI();

			const TIMESTAMP startTime = getTimeStamp();

			for (unsigned int n = 0; n < functionCount; n++)
			{
				processFunction(getn_func(n));

				if (!WaitBox::isUpdateTime())
					continue;

				if (!WaitBox::updateAndCancelCheck(static_cast<int>(static_cast<float>(n) / static_cast<float>(functionCount) * 100.0f)))
					continue;

				msg("* Aborted *\n");
				break;
			}

			WaitBox::hide();

			char count2[32] = "";
			snprintf(count2, sizeof(count2), "%lld", commentCount);

			msg("Done, generated %s string comments in %s.\n", count2, timeString(getTimeStamp() - startTime));
			msg("---------------------------------------------------------------------\n");
			refresh_idaview_anyway();

			return true;
		}

		warning("Auto analysis must finish first before you run this plug-in!");
		msg("\n*** Aborted ***\n");

		return false;
	}
	CATCH();

	return true;
}

// Remove whitespace & unprintable chars from the input string
static void filterWhitespace(char* pstr)
{
	char* ps = pstr;
	while (*ps)
	{
		// Replace unwanted chars with a space char
		char c = *ps;
		if ((c < ' ') || (c > '~'))
			*ps = ' ';
		ps++;
	}

	// Trim any starting space(s)
	ps = pstr;
	while (*ps)
	{
		if (*ps != ' ')
			break;
		memmove(ps, ps + 1, strlen(ps));
	}

	// Trim any trailing space
	ps = (pstr + (strlen(pstr) - 1));
	while (ps >= pstr)
	{
		if (*ps != ' ')
			break;
		*ps-- = 0;
	}
}

// static int __cdecl compare(const void* a, const void* b)
// {
// 	const auto sa = static_cast<const STRC *>(a);
// 	const auto sb = static_cast<const STRC *>(b);
//
// 	return sa->refs - sb->refs;
// }

static int __cdecl compare(const void *a, const void *b)
{
	STRC *sa = (STRC *)a;
	STRC *sb = (STRC *)b;
	return (sa->refs - sb->refs);
}

size_t getCharacterLength(int strtype, size_t byteCount)
{
	static const size_t bytesPerChar[] =
	{
		1, // #define ASCSTR_TERMCHR  0              ///< Character-terminated ASCII string.
		1, // #define ASCSTR_PASCAL   1              ///< Pascal-style ASCII string (one byte length prefix)
		1, // #define ASCSTR_LEN2     2              ///< Pascal-style, two-byte length prefix
		2, // #define ASCSTR_UNICODE  3              ///< Unicode string (UTF-16)
		2, // #define ASCSTR_UTF16    3              ///< same
		4, // #define ASCSTR_LEN4     4              ///< Pascal-style, four-byte length prefix
		2, // #define ASCSTR_ULEN2    5              ///< Pascal-style Unicode, two-byte length prefix
		2, // #define ASCSTR_ULEN4    6              ///< Pascal-style Unicode, four-byte length prefix
		4, // #define ASCSTR_UTF32    7              ///< four-byte Unicode codepoints
	};
	return byteCount / bytesPerChar[strtype];
}

// Process function
static void processFunction(func_t* f)
{
	const size_t MIN_STR_SIZE = 4;

	// Skip tiny functions for speed
	if (f->size() < 8)
		return;

	// Skip if it already has type comment
	// TODO: Could have option to just skip comment if one already exists?
	bool skip = false;

	qstring qBuf;

	if (get_func_cmt(&qBuf, f, true) == -1)
	{
		get_func_cmt(&qBuf, f, false);
	}
	
	// Ignore common auto-generated comments
	if (qBuf.size() > 0)
	{
		const auto tempComment = qBuf.extract();

		if (strncmp(tempComment, "Microsoft VisualC", SIZESTR("Microsoft VisualC")) != 0)
		{
			if (strstr(tempComment, "\ndoubtful name") == nullptr)
			{
				skip = true;
			}
		}

		// Clear buffer and free memory
		qfree(tempComment);
	}
	
	// TODO: Add option to append to existing comments?

	if (skip)
		return;

	// Iterate function body for string references from the function entry point
	unsigned int nStr = 0;
	func_item_iterator_t it(f);

	do
	{
		// Has an xref?
		const ea_t currentEA = it.current();
		xrefblk_t xb{};

		if (!xb.first_from(currentEA, XREF_DATA))
			continue;

		// Points to a string?
		if (!isString(xb.to))
			continue;

		// Get string type                       
		const int32 strtype = getStringType(xb.to);
		const size_t len = get_max_strlit_length(xb.to, strtype, ALOPT_IGNHEADS);

		if (getCharacterLength(get_str_type_code(strtype), len) <= static_cast<unsigned int>(MIN_STR_SIZE + 1))
			continue;

		// Will convert non-printable to characters to Pascal-style strings
		get_strlit_contents(&qBuf, xb.to, len, strtype, reinterpret_cast<size_t*>(qBuf.size()), STRCONV_ESCAPE);

		// Get string from buffer
		const auto contents = qBuf.extract();

		filterWhitespace(contents);

		if (!contents)
			continue;

		// If it's tiny, continue
		if (strlen(contents) < MIN_STR_SIZE)
			continue;

		// If already in the list, just update it's ref count
		bool skipRef = false;

		for (unsigned int j = 0; j < nStr; j++)
		{
			if (strcmp(stringArray[j].str, contents) != 0)
				continue;

			stringArray[j].refs++;
			skipRef = true;

			break;
		}

		if (skipRef)
			continue;

		// Add it to the list
		strncpy(stringArray[nStr].str, contents, MAX_LABEL_STR);
		qfree(contents);

		stringArray[nStr].refs = 1;
		++nStr;

		// Bail out if we're at max string count
		if (nStr >= MAX_LINE_STR_COUNT)
			break;
	}
	while (it.next_addr());

	// Got at least one string?
	if (!nStr)
		return;

	// Sort by reference count
	if (nStr > 1)
	{
		qsort(stringArray, nStr, sizeof(STRC), compare);
	}

	// Concatenate a final comment string
	char comment[MAX_COMMENT + MAX_LABEL_STR] = {"#STR: "};
	for (unsigned int i = 0; i < nStr; i++)
	{
		STRC* sc = &stringArray[i];
		auto freeSize = static_cast<unsigned int>(MAX_COMMENT - static_cast<int>(strlen(comment)) - 1);

		if (freeSize > static_cast<unsigned int>(6) && freeSize < static_cast<unsigned int>(strlen(sc->str) + 2))
			break;

		char temp[MAX_LABEL_STR];
		temp[SIZESTR(temp)] = 0;
		_snprintf(temp, SIZESTR(temp), "\"%s\"", sc->str);
		strncat(comment, temp, freeSize);

		// Continue line?
		if (static_cast<unsigned int>(i + 1) >= nStr)
			continue;

		freeSize = static_cast<unsigned int>(MAX_COMMENT - strlen(comment) - 1);

		if (freeSize <= static_cast<unsigned int>(6))
			continue;

		strncat(comment, ", ", freeSize);
	}

	// Add/replace comment
	set_func_cmt(f, "", true);
	set_func_cmt(f, "", false);
	set_func_cmt(f, comment, true);
	commentCount++;
}

// ============================================================================

// Plug-in description block
extern "C" ALIGN(16) plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION, // IDA version plug-in is written for
	PLUGIN_UNL,            // Plug-in flags
	IDAP_init,             // Initialization function
	IDAP_term,             // Clean-up function
	IDAP_run,              // Main plug-in body
	IDAP_comment,          // Comment - unused
	IDAP_help,             // As above - unused
	IDAP_name,             // Plug-in name shown in Edit->Plugins menu
	nullptr                // Hot key to run the plug-in
};
