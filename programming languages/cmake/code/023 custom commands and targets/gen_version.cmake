# Пишет version.h с текущей датой
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
file(WRITE ${OUT}
        "#pragma once\n"
        "#define BUILD_DATE \"${BUILD_DATE}\"\n"
        "#define BUILD_TYPE \"${BUILD_TYPE}\"\n"
)
