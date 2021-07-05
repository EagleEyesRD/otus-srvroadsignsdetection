cmake_minimum_required(VERSION 3.5)

project(srvroadsignsrecognition)

find_path(rabbitmq_INCLUDE_DIR
  NAMES amqp.h)

find_library(rabbitmq_LIBRARY
  NAMES rabbitmq)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(RabbitMQ DEFAULT_MSG
  rabbitmq_INCLUDE_DIR
  rabbitmq_LIBRARY)

include_directories(AMQP SignDetector)
set(SOURCES JSONBase.cpp FrameOptions.cpp CommandLine.cpp)
set(HEADER_FILES AMQP/AMQPcpp.h SignDetector/SignDetector.h SignDetector/ImageConverter.h)

add_executable(srvroadsignsrecognition main.cpp JSONBase.cpp FrameOptions.cpp CommandLine.cpp
  SignDetector/SignDetector.h
  SignDetector/SignDetector.cpp 
  SignDetector/ImageConverter.h
  SignDetector/ImageConverter.cpp
  AMQP/AMQPcpp.h
  AMQP/AMQPQueue.cpp
  AMQP/AMQPMessage.cpp
  AMQP/AMQPExchange.cpp
  AMQP/AMQPException.cpp
  AMQP/AMQPBase.cpp
  AMQP/AMQP.cpp 
)

target_link_libraries(srvroadsignsrecognition rabbitmq.4.lib librabbitmq.4.lib )
set_target_properties(srvroadsignsrecognition PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)


