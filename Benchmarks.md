## System

OS: Linux Fedora 6.16

CPU: Intel(R) Core(TM) i7-10750H CPU @ 2.60GHz

RAM: 8 GB

## Test data

Test data can be found in `images/input`.

### "Small" images:

Mona_Lisa.bmp 960x1431 pixels

Almond_van_Gogh.bmp 1367x1080 pixels

Sunflowers_van_Gogh.bmp 960x1211 pixels

Impression_Sunrise.bmp 1392x1080 pixels

### "Big" images:

The_Ninth_Wave.bmp 3215x2160 pixels

## Testing

### Functions to test

First of all, there are three functions to test:
1. First one is applying a filter to an image sequentually.
2. Second one uses one thread to read the image, then **n** worker threads apply the filter concurrently, then one thread to write the image.
3. Third one uses different kinds of threads and queues to process an array of images concurrently.

### Ways to divide work between worker threads

3 ways to divide work between worker threads were chosen:
1. Rowwise - work is divided by rows
2. Pixelwise - work is divided on pixels
3. Pilewise - work is divided into piles

### What these tests measure

In case of single image convolution, these tests measure average time of full image convolution cycle: reading the source image, applying the filter to it, writing updated image to memory.

In case of multiple images, these tests measure average time between reading the first image from memory and writing the last image to the memory.

## Single image

In this test, workers number n=4.

### Small:

![Alt text](images/bench_res/benchmark_results_1s.png)

### Big: 
![Alt text](images/bench_res/benchmark_results_1b.png)


It can be seen that sequential algorithm is far slower than any of the parallel ones.

The difference between any parallel scenario is almost negligible.

## Multiple images

For this test, 4 images were chosen to be processed in a sequence. For function that uses queues, all of readers, writers and foremen (workers' orchestrators) were set to be 4, to begin proccessing tasks as soon as possible.

Workers number is still n=4.

### Small:

![Alt text](images/bench_res/benchmark_results_4s.png)

### Big:

![Alt text](images/bench_res/benchmark_results_4b.png)


As we can see, as images number is 4 times larger, sequential processing time is almost 4 times larger.

For parallel (blue) scenarios, time of their processing increased in more than 4 times. This is supposedly due to them having stalls, because they can't do different stages of processing in parallel and need to wait before moving on to the next stage.

For parallel + queue (green) scenarios, time of their processing has roughly tripled.
Comparing to parallel implementations without queues, those using them have 1.5-2x times speedup when processing an array of 4 images.

## Conclusion

Parallel processing accelerated BMP image convolution by more than a factor of three. These gains can be increased further by employing specialized threads and queues. For instance, when processing a sequence of four images, this technique yielded a 1.5-2x speedup over standard parallel methods. Performance is expected to improve even more with a greater number of images.