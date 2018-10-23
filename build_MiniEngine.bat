cd ./ThirdParty
cd DirectX-Graphics-Samples
git pull
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\IDE\devenv" .\MiniEngine\ModelViewer\ModelViewer_VS15.sln /Build "Debug|Windows"
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\IDE\devenv" .\MiniEngine\Core\Core_VS15.vcxproj /Build "Release|x64"
pause