# Prerequisites

- Microsoft Visual Studio 2013 SP2 and above - I use VS 2022 community version.
- [CUDA toolkit v12.1.0]("https://developer.nvidia.com/cuda-toolkit-archive").(Only need when we compile FFmpeg,not needed to run the FFmpeg compiled binaries).
- git tools - I use [Git for Windows]("https://gitforwindows.org/").
- [msys2]("").

## Compiling for Windows

- create an empty folder as your working directory. eg:"d:/ffmpeg".
- clone ffnvcodec to working directory

```-git
git clone https://git.videolan.org/git/ffmpeg/nv-codec-headers.git
```

- clone FFmpeg to working directory
  
```-git
git clone https://git.ffmpeg.org/ffmpeg.git
```

- create folder nv_sdk in working directory
- copy header files to nv_sdk from "D:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1/include".
- copy library files to nv_sdk from "D:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1/lib/x64".
- Launch  Visual Studio x64 Native Tools Command Prompt.You can find it here : **C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Visual Studio 2022\Visual Studio Tools\VC** or here : **C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat**
