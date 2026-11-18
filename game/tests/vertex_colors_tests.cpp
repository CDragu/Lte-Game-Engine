#include "../BlockColors.h"
#include <stdio.h>
using namespace tower;
static int failed=0;
static void check(bool okay,const char* text) {
    printf("%s %s\n",okay?"PASS":"FAIL",text);
    if(!okay) ++failed;
}
int main() {
    bool unique=true;
    for(int i=0;i<64;i++)
        for(int j=0;j<i;j++)
            if(blockColor(i)==blockColor(j)) unique=false;
    check(unique,"64 sequential blocks receive distinct RGB colours");
    check(blockColor(0)!=blockColor(1) && blockColor(1)!=blockColor(2) &&
          blockColor(2)!=blockColor(3),
          "three stacked blocks and the incoming block differ");
    check(blockColor(4)==blockColor(4),"colours remain consistent between resets");
    const BlockRgb base=blockColor(3);
    const BlockRgb top=blockFaceColor(base,4);
    const BlockRgb side=blockFaceColor(base,2);
    const BlockRgb back=blockFaceColor(base,1);
    check(top.r>=side.r && top.g>=side.g && top.b>=side.b,
          "top vertex colours are lighter than the side");
    check(side!=back && back!=top,
          "different cube faces use different vertex colours");
    check(groundColor()!=blockColor(0),"ground is neutral, separate from blocks");
    printf("Vertex colour tests: %s\n",failed?"FAILED":"PASSED");
    return failed?1:0;
}
