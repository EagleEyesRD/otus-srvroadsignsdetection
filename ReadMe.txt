Description:

Project has been creating as microservice.
It is detection & recognition road signs wich get from image. 
Images load other services and create and jsons push to RabbitMQ as events on them. 
Our project gets packets with jsons, it parses and loads each image on HDD. 
After that it detects color blobs and check them by neuronetwork with tensorflow.
Then it creates json with special properties and push it to RabbitMQ again.  

Configuration:

Library OpenCV is core this project, but it not always, because we need to do lightweight solve.
It depends on  difine use_cvtColor 1  in ImageConverter.h.
There are created special method to convert image from YUV420 to BGR(cv::Mat from OpenCV).
It's long way to get good quality because inline formules OpenCV make this convertion not correct.


Depends:
vcpkg

install as packets of vcpkg:
OpenCV( ver.4.5.2)
rabbitmq-c
amqp
rapidjson

 


