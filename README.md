# IDA7 FunctionStringAssociate

Original plugin developed by sirmabus, ported to IDA 7.0

# Compiling Notes

The following system environment variables should be set before running Visual Studio:

1. `$(IDADIR)` - path to IDA 7.0 where ida64.exe can be found
2. `$(IDASUPPORT)` - path to [IDA7-SupportLib](https://github.com/ecx86/IDA7-SupportLib)
3. `$(IDAWAITBOX)` - path to [IDA7-WaitBoxEx](https://github.com/dude719/IDA_WaitBoxEx-7.0)

Or, you can edit them in the Property Manager under PropertySheet.props

The IDA 7.0 SDK should also be located at `$(IDADIR)\idasdk`.

# Credits

- **sirmabus** for creating everything
- **ecx86** for porting SupportLib
- **dude719** for porting WaitBoxEx
