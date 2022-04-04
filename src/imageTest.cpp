#include "Image.hpp"

int main(){
    Image img(1280,720);
    for(int h=0;h<720;h++){
        for(int w=0;w<1280;w++){
            img.set(w,h,glm::vec3(
                ((float)w)/1280.0f,
                ((float)h)/720.0f,
                0.2f
            ));
        }
    }
    img.exportImage("test.png");
    //img.exportImage("test.bmp");

    return 1;
}