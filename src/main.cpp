#include <FreeImage.h>
#include <iostream>

int main() {
    FreeImage_Initialise();
    
    std::cout << "FreeImage version: " << FreeImage_GetVersion() << std::endl;
    
    FreeImage_DeInitialise();
    
    return 0;
}