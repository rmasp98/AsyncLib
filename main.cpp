#include "AsyncLib/logger.hpp"

int main() {
  auto logger = async_lib::GetLogger();
  logger.Error("Test");
  
}

