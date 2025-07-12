//
// Created by mohammad on 5/20/23.
//
#include <iostream>
#include <memory> 

#include <aixlog.hpp>

#include "common.h"

#undef LOGGER
#define LOGGER(level) LOG(level) << TAG("main") << " "


int main(int argc, char** argv){
    AixLog::Log::init(
        {
            std::make_shared<AixLog::SinkCout>(AixLog::Severity::trace, "cout: %Y-%m-%d %H-%M-%S.#ms [#severity] (#tag) #message"),

        }
    );

    LOGGER(INFO) << "Hello, world!";
    LOGGER(WARNING) << "This is a warning message";
    LOGGER(ERROR) << "This is an error message";
    LOGGER(FATAL) << "This is a fatal message";


    printf("\ngood bye :)\n");
		
    return 0;
}
