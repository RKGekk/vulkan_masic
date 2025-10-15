
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)         machine=Linux;;
    Darwin*)        machine=Mac;;
    CYGWIN*)        machine=Cygwin;;
    MINGW64_NT*)    machine=MinGw64;;
    MINGW32_NT*)    machine=MinGw32;;
    MSYS_NT*)       machine=MSys;;
    *)              machine="UNKNOWN:${unameOut}"
esac
if [ "$machine" == "Mac" ]; then
    $VK_SDK_PATH/macOS/bin/glslc shader.vert -o vert.spv
    $VK_SDK_PATH/macOS/bin/glslc shader.frag -o frag.spv
elif [ "$machine" == "Linux" ]; then
    $VK_SDK_PATH/Bin/glslc shader.vert -o vert.spv
    $VK_SDK_PATH/Bin/glslc shader.frag -o frag.spv
elif [ "$machine" == "MinGw64" ]; then
    $VK_SDK_PATH/Bin/glslc shader.vert -o vert.spv
    $VK_SDK_PATH/Bin/glslc shader.frag -o frag.spv
fi