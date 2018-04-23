/*cppimport
<%
cfg['compiler_args'] = ['-std=c++11']
cfg['libraries'] = ['vncclient']
cfg['include_dirs'] = ['/usr/include/eigen3/']
setup_pybind11(cfg)
%>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <cstdint>
#include <tuple>
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <rfb/rfbclient.h>

namespace py = pybind11;

static char *password = NULL;

char *GetPassword(rfbClient *client) {
  return password;
}

class Helper {
 private:
  std::string port_;
  rfbClient* client_;
  
 public:
  Helper(const std::string &port) : port_(port) {
    client_ = rfbGetClient(8, 3, 4);

    if(password == NULL) {
      password = (char *)malloc(8);
      strcpy(password, "rabbits");
      password[7] = 0;
    }

    client_->GetPassword = GetPassword;

    int argc = 2;
    char *argv[2] = { (char *)NULL, (char *)port.c_str() };
    if(!rfbInitClient(client_, &argc, argv)) {
      throw std::runtime_error("Failed to initialize client");
    }
  }

  ~Helper() {
    rfbClientCleanup(client_);
  }

  void handleMessage() {
    int s = WaitForMessage(client_, 0);

    if(s == 0)
      return;

    if(s < 0)
      throw std::runtime_error("Error waiting for message");

    if(!HandleRFBServerMessage(client_))
      throw std::runtime_error("Message read failed");
  }

  std::tuple<int, int, int> getScreenDims() {
    return std::make_tuple(client_->height, client_->width, 3);
  }

  Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> getScreen() {
    Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> vec(client_->height * client_->width * 3);

    rfbPixelFormat* pf = &client_->format;
    int bpp = pf->bitsPerPixel / 8;
    int row_stride = client_->width * bpp;

    for (int j = 0; j < client_->height; ++j) {
      for (int i = 0; i < client_->width; ++i) {
        unsigned char* p = client_->frameBuffer + j * row_stride + i * bpp;
        unsigned int v;
        
        if(bpp == 4)
          v = *(unsigned int*)p;
        else if(bpp==2)
          v = *(unsigned short*)p;
        else
          v = *(unsigned char*)p;

        vec[j * client_->width * 3 + i * 3 + 0] = (v >> pf->redShift) * 256 / (pf->redMax + 1);
        vec[j * client_->width * 3 + i * 3 + 1] = (v >> pf->greenShift) * 256 / (pf->greenMax + 1);
        vec[j * client_->width * 3 + i * 3 + 2] = (v >> pf->blueShift) * 256 / (pf->blueMax + 1);
      }
    }

    return vec;
  }
};

PYBIND11_MODULE(ptest, m) {
  py::class_<Helper>(m, "Helper")
      .def(py::init<const std::string &>())
      .def("handleMessage", &Helper::handleMessage)
      .def("getScreenDims", &Helper::getScreenDims)
      .def("getScreen", &Helper::getScreen);
}
