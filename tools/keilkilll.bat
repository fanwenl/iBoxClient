cd ..
del *.bak /s
del *.ddk /s
del *.edk /s
del *.lst /s
del *.lnp /s
del *.mpf /s
del *.mpj /s
del *.obj /s
del *.omf /s
::del *.opt /s  ::������ɾ��JLINK������
del *.plg /s
del *.rpt /s
del *.tmp /s
del *.__i /s
del *.crf /s
del *.o /s
del *.d /s
del *.axf /s
del *.tra /s
del *.dep /s           
del JLinkLog.txt /s
del *.a /s

del *.iex /s
::del *.htm /s
del *.sct /s
del *.map /s
del *.hex /s
del *.hex /s
del *.dbgconf /s
del *.scvd /s
::rd 删除文件夹以及文件/不需要确认
rd .\Project\DebugConfig\ /s/q
rd .\Project\Listings\ /s/q
rd .\Project\Objects\ /s/q
exit
