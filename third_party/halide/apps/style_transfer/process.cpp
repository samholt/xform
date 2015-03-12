#include <stdio.h>
#include "local_laplacian.h"
#include "static_image.h"
#include "image_io.h"
#include <sys/time.h>

int main(int argc, char **argv) {
    if (argc < 6) {
        printf("Usage: ./process input.png levels alpha beta output.png\n"
               "e.g.: ./process input.png 8 1 1 output.png\n");
        return 0;
    }

    Image<float> input = load<float>(argv[1]);
    int levels = atoi(argv[2]);
    float alpha = atof(argv[3]), beta = atof(argv[4]);
    Image<float> output(input.width(), input.height(), 3);

    // Timing code
    timeval t1, t2;
    unsigned int bestT = 0xffffffff;
    for (int i = 0; i < 1; i++) {
      gettimeofday(&t1, NULL);
      local_laplacian(levels, alpha/(levels-1), beta, input, output);
      gettimeofday(&t2, NULL);
      unsigned int t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
      if (t < bestT) bestT = t;
    }
    printf("%u\n", bestT);


    local_laplacian(levels, alpha/(levels-1), beta, input, output);

    save(output, argv[5]);

    return 0;
}
