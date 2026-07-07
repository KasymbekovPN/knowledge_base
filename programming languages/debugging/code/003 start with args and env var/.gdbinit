file .\build\debug\app.exe
set args one two --flag=42
set environment LOG_LEVEL=debug
set environment BOOST_ASIO_DISABLE_THREADS=1
break main.cpp:21
run
print argv[1]
print argv[2]
print argv[3]
print slog_level
print s_threads