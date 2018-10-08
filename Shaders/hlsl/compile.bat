"C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\dxc" -T vs_6_0 -E VS Simple.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\dxc" -T vs_6_0 -E VS Color.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\dxc" -T vs_6_0 -E VS 000test.hlsl -Fo vs.cso
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x86\dxc" -T ps_6_0 -E PS Simple.hlsl -Fo ps.cso
pause