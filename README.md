# 01.一切的开始|Bresenham算法绘制线段

## 本项目代码已托管至github，将会随着博客实时更新进度

每一节的工程我都会创建一个新的分支，分支名由这一节的数字决定。

https://github.com/chiuhoukazusa/LearningTinyrenderer/tree/01

## 前言

看着一片黑屏的渲染器和空空如也的cpp文件，我们可能无从下手，那就让我们回忆一下相关图形学知识。

我们都知道模型是由三角面构成的，三角面首先得有三角形，三角形又得有三条边。所以绘制线段就是我们要做的第一步。

## 绘制一条从(x0, y0)到(x1, y1)的线段

因为我们是在像素上操作，所以我们就将传进来的这四个参数当做是int类型处理。

我们都知道直线拥有一个点斜式方程。
$$
y = kx + b
$$
我们能得到y的增量和x的增量之间的关系
$$
△y = k△x
$$
那么我们让x每次增加一个像素的距离，这样每次△y就是一个固定的值，加上后让y再四舍五入到对应的像素点，把像素点画出来就行。

听起来很简单，实现起来也很简单。

```c++
void rst::rasterizer::draw_line(int x0, int y0, int x1, int y1) {
	float slope = (float)(y1 - y0)/ (float)(x1 - x0);
	int x, y, dx, dy, i;
	x = x0;
	y = y0;
	image.set(x, y, red);
	for (i = x0; i <= x1; i++) {
		x = i;
		dx = x - x0;
		dy = (int)(slope * dx);
		y = dy + y0;
		image.set(x, y, red);
	}
}
```

![0101第一条线段](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/202208251744045.png)

我们成功了！我们输入的测试用例是从(13, 14)到(78, 25)的一条线段，看起来我们的代码工作的很好。

但细心的朋友会发现我们的代码有着很多的问题，我们默认x0小于x1，排除了斜率为负数的情况，而且即使斜率为正数，当我们的斜率大于1时，也就是当这条直线变得陡峭时，很显然会发生问题。

![img](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/0102算法缺陷.png)

我们在原本线段的基础上又画了一条从(13, 14)到(25, 78)的直线，很明显，因为我们是基于x来推算y，当y的增速快于x的时候，我们的直线就只能是这么一些离散的点。

除此之外，我们的代码还有更多的缺点。我们引入了一个浮点型变量slope用以存储直线的斜率，但是浮点运算总是慢的。由于线段绘制在一个渲染器中作为一个最基础的功能将会被调用很多很多次，如果我们可以只用整型值来运算的话，对于整个渲染器的效率而言将会是非常显著的提升。

## 跟斜率说再见

让我们回顾我们的算法，其实绘制线段的本质难点就在于四舍五入。

为了研究方便，我们先只讨论斜率位于(0,1)的情况。

在这种情况下，当x增加1的时候，因为y的增速小于x，y的增长范围是(0,1)，因此每一个x值对应一个y的值。当y的增量>0.5时，下一个像素的y坐标+1，否则下一个像素的y坐标不变。用通俗点的话来说，下一个像素只会出现在上一个像素的右边或者是右上方两个位置。

再让我们关注从第一个像素到第二个像素之间发生了什么。第一个像素坐标是(x0, y0)，第二个像素坐标是(x0 + 1, y?)。y?的值可能是y0或者y0 + 1，这取决于这条直线的斜率。我们用slope来存储斜率这个变量，由于x的增量为1。即：
$$
y? = (1 * slope) > 0.5 ? y0 + 1 : y0;
$$
由于slope = (y1 - y0) / (x1 - x0) = dy/dx。我们将dy和dx带入一下，表达式的值就会变成：
$$
y?=(2*dy-dx>0)?y0+1:y0;
$$
很好，我们已经消除了slope这个可恨的浮点型变量，那么让我们继续递推第三个像素的位置。

我们发现第三个像素的判断表达式将取决于上一个像素是生成在哪个位置，那么我们分两种情况讨论：

###### 如果上一个像素的y坐标递增了1

那么我们将要判断2 * slope > 1.5这个表达式的真假，我们继续用dx和dy来代替slope，这个表达式就变成了4 * dy - 3 * dx > 0。我们将表达式稍微分解一下，变成：
$$
2*dy-dx+(2*dy-2*dx)<0
$$
我们在推导中会发现，dy之所以前面系数多了2，是因为我们这是第三个像素，斜率前的值从1变成了2。而dx之所以系数多了2，则是因为我们上一个像素递增了1，原式右边的值从0.5变成了1.5增加了1。这一切都是有迹可循的，因此我们可以总结在这种情况下，我们表达式的左值需要增加(2 * dy - 2 * dx)。

###### 如果上一个像素的y坐标不变

那么我们将要判断2 * slope > 0.5这个表达式的真假，即
$$
2*dy-dx+2*dy<0
$$
因为我们原式的右边还是0.5，所以dx的系数不变，我们只需增加2 * dy。

好了，有了以上这种思路，我们就已经可以构造一个循环了：

```c++
void rst::rasterizer::draw_line(int x0, int y0, int x1, int y1) {
	int x, y, dx, dy, px, i;
	x = x0;
	y = y0;
	dx = x1 - x0;
	dy = y1 - y0;
	px = 2 * dy - dx;
	image.set(x, y, red);
	for (i = x0 + 1; i <= x1; i++) {
		x = i;
		if (px > 0) {
			y++;
			px += 2 * dy - 2 * dx;
		}
		else
		{
			px += 2 * dy;
		}
		image.set(x, y, red);
	}
}
```

将(13, 14, 78, 25)做测试用例传进函数，我们的运行结果如下图，和第一张图画出来的一样！但是我们没有用到任何浮点数运算！

![img](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/0101第一条线段.png)

到这里，我们已经成功解决了浮点数这个难题，将斜率清除出了我们的函数实现，但是我们依旧没有解决当斜率不处于(0, 1)时的问题。

## 开始考虑其他情况

###### 斜率>1的情况

也就是dy>dx>0的时候，这时候y的增速大于x，一个x不再对应一个确定的y。但是一个y对应了一个确定的x，我们完全可以用之前的算法，只需将x和y的立场对调就行，记得再定义一个py来对应上一个代码的px。

```
void rst::rasterizer::draw_line(int x0, int y0, int x1, int y1) {
	int x, y, dx, dy, px, py, i;
	x = x0;
	y = y0;
	dx = x1 - x0;
	dy = y1 - y0;
	px = 2 * dy - dx;
	py = 2 * dx - dy;
	image.set(x, y, red);
	if (dy < dx) {
		for (i = x0 + 1; i <= x1; i++) {
			x = i;
			if (px > 0) {
				y++;
				px += 2 * dy - 2 * dx;
			}
			else
			{
				px += 2 * dy;
			}
			image.set(x, y, red);
		}
	}
	else
	{
		for (i = y0 + 1; i <= y1; i++) {
			y = i;
			if (py > 0) {
				x++;
				py += 2 * dx - 2 * dy;
			}
			else
			{
				py += 2 * dx;
			}
			image.set(x, y, red);
		}
	}
}
```

![img](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/0103斜率大于1.png)

还记得之前我们给的一张绘制失败的图片吗，现在我们传入相同的测试用例，已经可以正常运作了。

###### 斜率为负数的情况

也就是dx和dy不同号的时候，当x或者y增加1的时候，相对应的另一个坐标由递增或不变这两种情况变成了递减或不变这两种情况，也就是说我们只用加上一个判断语句就可以实现。同时我们也要考虑我们不按x0<x1, y0<y1的格式来输入的情况，那就继续加判断语句。值得一提的是，因为我们添加了好几个判断语句，为了循环能够继续运作我们需要修改一下循环条件，具体实现如下文：

```c++
void rst::rasterizer::draw_line(int x0, int y0, int x1, int y1) {
	int x, y, dx, dy, dxa, dya, px, py, xb, yb, i;
	x = x0;
	y = y0;
	dx = x1 - x0;
	dy = y1 - y0;
	dxa = abs(dx);
	dya = abs(dy);
	px = 2 * dya - dxa;
	py = 2 * dxa - dya;
	image.set(x, y, red);
	if (dya < dxa) {
		if (dx > 0) {
			x = x0;
			y = y0;
			xb = x1;
		}
		else
		{
			x = x1;
			y = y1;
			xb = x0;
		}
		for (i = 0; x <= xb; i++) {
			x++;
			if (px > 0) {
				if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
					y++;
				}
				else
				{
					y--;
				}
				px += 2 * dya - 2 * dxa;
			}
			else
			{
				px += 2 * dya;
			}
			image.set(x, y, red);
		}
	}
	else
	{
		if (dy > 0) {
			x = x0;
			y = y0;
			yb = y1;
		}
		else
		{
			x = x1;
			y = y1;
			yb = y0;
		}
		for (i = 0; y <= yb; i++) {
			y++;
			if (py > 0) {
				if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
					x++;
				}
				else
				{
					x--;
				}
				py += 2 * dxa - 2 * dya;
			}
			else
			{
				py += 2 * dxa;
			}
			image.set(x, y, red);
		}
	}
}
```

我们输入测试用例：

```C++
...    
	rst.draw_line(0, 50, 100, 50);
    rst.draw_line(50, 0, 50, 100);
    rst.draw_line(50, 50, 60, 0);
    rst.draw_line(50, 50, 40, 0);
    rst.draw_line(50, 50, 0, 40);
    rst.draw_line(50, 50, 0, 60);
    rst.draw_line(50, 50, 100, 60);
    rst.draw_line(50, 50, 60, 100);
    rst.draw_line(50, 50, 100, 40);
    rst.draw_line(50, 50, 40, 100);
...
```

最终得到下图：

![](https://raw.githubusercontent.com/chiuhoukazusa/blog_img/main/0104final.png)

可以看到测试结果非常的振奋人心，我们成功绘制出了10条斜率各不相同（甚至有条线斜率不存在）的线段。

至此，我们已经成功实现了一个传入两个坐标值即可连线的算法，并且只使用了整型运算。

以上，即是bresenham算法的实现，这个算法至今还在被广泛地使用着，除非哪天开发出了浮点数运算比整型运算更快的硬件，不然这个算法将依旧被使用着作为我们渲染计算机世界的基础。
