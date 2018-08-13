# checkers

[![](https://i.imgur.com/a0tAgwl.png?2)](http://www.youtube.com/watch?v=9Maesq2iISM) ![](https://i.imgur.com/oMSBPvL.png?2) ![](https://i.imgur.com/FrdskvG.png?3)

Model a game of checkers from a live video feed where the **camera is not static**.

## Algorithm

1. Sharpen and equalize histogram to find corners more easily
2. `cv::findChessboardCorners` and manually compute the exterior corners `completeBoardCorners`
3. Median blur to obtain better color segmentation results
4. Red segmentation, yellow segmentation
5. Gaussian blur to obtain better results with the Hough Circle transform
6. Apply the Hough Circle transform to the color segmentations to find all the red and yellow circles
7. Check if the circles’ center is (reasonably) inside the 4 corners that define a game position

## Development

```bash
$ cmake . && make
$ ./checkers bin/resized.mp4
```
