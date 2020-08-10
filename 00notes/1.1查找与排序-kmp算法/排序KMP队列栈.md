# 十种算法，KMP，队列，栈，堆
<font face="黑体">我是黑体字</font>
<font face="微软雅黑">我是微软雅黑</font>
<font face="STCAIYUN">我是华文彩云</font>
<font color=red>我是红色</font>
<font color=#008000>我是绿色</font>
<font color=Blue>我是蓝色</font>
<font size=5>我是尺寸</font>
<font face="黑体" color=green size=5>我是黑体，绿色，尺寸为5</font>
## 十种排序算法分析
<font color=Blue>1：冒泡排序：</font>   </br>
	把最大/最小的与相邻的一直做对比，确定最后一个是最大/最小。</br><br/>
	平均时间复杂度： Q(n*n)</br><br/>
	空间复杂度：    Q(1)</br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>2：选择排序：</font></br><br/>
	选择出最大/最小的，交换放对位置。  如：先确定第一个，再确定第二个。。。</br><br/>
	平均时间复杂度： Q(n*n)</br><br/>
	空间复杂度：    Q(1)</br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>3：插入排序：</font></br><br/>
	将数据插入到已经排序好的队列中==>把第一个元素看做是只有一个值得有序队列。</br><br/>
	平均时间复杂度： Q(n*n)</br><br/>
	空间复杂度：    Q(1)</br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>4：希尔排序：</font></br>
	按增量进行分组，排序。 （先取1/2,再取1/4,1/8.。。最后直到一个元素，排序。。。） ==》直接插入排序的改进(不稳定的)</br><br/>
	平均时间复杂度： Q(n^1/3)</br><br/>
	空间复杂度：    Q(1)</br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>5:归并排序: </font> 分治递归</br><br/>
	先拆分，直到拆分剩两个的比较，最后再合并。 </br><br/>
	平均时间复杂度： Q(n*log^n)</br><br/>
	空间复杂度：    Q(1)  </br><br/>
<font color=red>流程梳理：</font></br><br/>
<font color=Blue>6:快速排序： </font> 分治</br><br/>
	取临界值，小于该值得放左边，大于该值得放右边。  ==》确定了这个值得位置，排序其两边的。 ==》三种快排，四种优化</br><br/>
	三种快排： 选取固定位置为基值/随机选取基值/三个数取中间的为基值（第一个  中间  最后一个）</br><br/>
	四种优化：当待排序序列的长度分割到一定大小后，使用插入排序（对于很小和部分有序的数组，快排不如插排好。）</br><br/>
			在一次分割结束后，可以把与Key相等的元素聚在一起，继续下次分割时，不用再对与key相等元素分割</br><br/>
			优化递归操作。</br><br/>
			使用并行或多线程处理子序列。</br><br/>
	平均时间复杂度： Q(n*log^n)</br><br/>
	空间复杂度：    Q(log^n)  	</br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>7:堆排序: </font>构建大堆顶或者小堆顶，与最前面或者最后面的交换</br><br/>
	平均时间复杂度： Q(n*log^n)</br><br/>
	空间复杂度：    Q(1) </br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>8：计数排序： </font></br>根据数据下标进行排序==》用bit位去标识，然后依次遍历bit位，根据下标去存。</br><br/>
	平均时间复杂度： Q(n+k)  ---》k为整数的范围</br><br/>
	空间复杂度：    Q(k) </br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>9：桶排序/箱排序：</font>划分多个范围相同的区域，然后各自排序，最后归并。</br><br/>
	快速排序 ===》分成两个桶，再分   ==》内排序</br><br/>
	每个桶中放一个数据，就是计数排序===》对计数排序的优化 ==》外排序</br><br/>
	平均时间复杂度： Q(n+k)  ---》k为整数的范围</br><br/>
	空间复杂度：    Q(n+k) </br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>10：基数排序：</font>先按照个位排序，再十位，百位，千位。。。</br><br/>
	平均时间复杂度： Q(n*k)  </br><br/>
	空间复杂度：    Q(n+k) </br><br/>
<font color=red>流程梳理：</font></br><br/>

<font color=Blue>11：KMP算法： </font></br>
	1：字符串匹配算法，根据目标串回溯构建最大公共元素单元，实现查找优化。</br><br/>
	2：KMP的使用场景：</br><br/>

<font color=Blue>12：队列：</font>先进先出，队顶进，队尾出</br><br/>
	队列的结构定义：</br><br/>

<font color=Blue>13：栈： </font>栈顶进，栈顶出，按照一定的倍数申请栈大小。</br><br/>
	栈的结构定义：</br><br/>

<font color=Blue>14：二叉树：</font> (在某种情况下，二叉树退化为链表)</br><br/>
	实现业务与数据分离结构，二叉树的定义：</br><br/>

<font color=Blue>16：堆：</font>用数组展示的二叉树。 ===>最短路径用到</br><br/>
		1：构建优先队列（数据被赋予访问级别）</br><br/>
		2：支持堆排序</br><br/>
		3：快速找出集合中的最小/最大值</br><br/>
