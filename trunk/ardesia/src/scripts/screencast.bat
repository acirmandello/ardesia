@echo off
rem Uncomment this to use the live screencast on icecast
rem To start the live stream successfully you must configure
rem the configuration file ezstream_stdin_vorbis.xml
rem as you desire to point to the right running icecast server
rem The configuration file is setted to work properly
rem if it is used a localhost icecast server with the default 
rem configuration
rem Client side you must have the ezstream installed

set ICECAST=FALSE
rem set ICECAST=TRUE

rem You must configure the right password; I put the default one
set ICECAST_PASSWORD=hackme
set ICECAST_ADDRESS=127.0.0.1
set ICECAST_PORT=8000
set ICECAST_MOUNTPOINT=ardesia.ogg
rem replace this with the name of the micropphone direct show audio input device
set AUDIO_DEVICE=default

set RECORDER_PROGRAM=vlc.exe
set RECORDER_PROGRAM_OPTIONS=-vvv -I dummy --dummy-quiet screen:// --screen-fps=12 :input-slave=dshow:// :dshow-vdev="none" :dshow-adev="%AUDIO_DEVICE%" --sout  "#transcode{venc=theora,vcodec=theo,scale=0.7,acodec=vorb,channels=2,samplerate=44100,deinterlace,audio-sync}:standard{access=file,mux=ogg,dst=%2}"
set RECORDER_AND_FORWARD_PROGRAM_OPTIONS=-vvv -I dummy --dummy-quiet screen:// --screen-fps=12 :input-slave=dshow:// :dshow-vdev="none" :dshow-adev="%AUDIO_DEVICE%" --sout  "#transcode{venc=theora,vcodec=theo,scale=0.7,acodec=vorb,channels=2,samplerate=44100,deinterlace,audio-sync}:duplicate{dst=std{access=shout,mux=ogg,dst=source:%ICECAST_PASSWORD%@%ICECAST_ADDRESS%:%ICECAST_PORT%/%ICECAST_MOUNTPOINT%},dst=std{access=file,mux=ogg,dst=%2}}"

if ""%1"" == ""start"" goto start

if ""%1"" == ""stop"" goto stop

:start
rem This start the recording on file
if "%ICECAST%" == "TRUE" goto icecast_start
rem if not icecast then only record the screencast
echo Start the screencast running %RECORDER_PROGRAM%
echo With arguments %RECORDER_PROGRAM_OPTIONS%
rem exec recorder
start %RECORDER_PROGRAM% %RECORDER_PROGRAM_OPTIONS%
goto end

:icecast_start
echo Start the screencast running %RECORDER_PROGRAM%
echo With arguments %RECORDER_AND_FORWARD_PROGRAM_OPTIONS%
rem exec recorder
start %RECORDER_PROGRAM% %RECORDER_AND_FORWARD_PROGRAM_OPTIONS%
goto end

:stop
set RECORDER_PID=cat %RECORDER_PID_FILE%
echo Stop the screencast killing %RECORDER_PROGRAM% 
TASKKILL /F /IM %RECORDER_PROGRAM%
goto end

:end
