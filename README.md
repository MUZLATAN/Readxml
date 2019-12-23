# 使用rapidxml解析,使用folly::dynamic序列化输出json

# 使用rapidxml解析,使用folly dynamic序列化输出json
> Folly是Facebook的一个重型库, rapidxml是一个轻量级的效率极高的xml解析器

__目的:__ 将如下的xml中部分字段写入到json文件
*`49.xml`*
```xml
<annotation>
	<folder>new</folder>
	<filename>49.jpg</filename>
	<path>C:\Users\HP\Desktop\new\49.jpg</path>
	<segmented>0</segmented>
	<object>
		<name>trafficcone</name>
		<pose>Unspecified</pose>
		<truncated>0</truncated>
		<difficult>0</difficult>
		<bndbox>
			<xmin>34</xmin>
			<ymin>48</ymin>
			<xmax>212</xmax>
			<ymax>281</ymax>
		</bndbox>
	</object>
</annotation>
```
*`target.json`*
```json

[
    {
        "_id": "5dfb4374df228c005d2e85c8",
        "md5": "25d7e5962a172b37364304a7c76131ab",
        "filename": "20191212/25d7e5962a172b37364304a7c76131ab",
        "ext": "jpg",
        "targets": [
             {
                "_id": "5dfb4374df228c005d2e85ca",
                "name": "traffic_cone",
                "type": "RECT",
                "data": {
                    "xmin": 0.972875,
                    "ymin": 0.8041666666666667,
                    "width": 0.009499999999999998,
                    "height": 0.049388888888888885
                }
            },
            {
                "_id": "5dfb4374df228c005d2e85ca",
                "name": "traffic_cone",
                "type": "RECT",
                "data": {
                    "xmin": 0.972875,
                    "ymin": 0.8041666666666667,
                    "width": 0.009499999999999998,
                    "height": 0.049388888888888885
                }
            }
        ],
        "tags": [
            "traffic_cone"
        ],
        "others": {
            "annotator": "gaow",
            "creator": "zhangfx",
            "createdAt": "2019-12-12T02:29:28.111Z",
            "updatedAt": "2019-12-19T09:30:42.922Z"
        }
    }
]
```
思路很简单, 先解析xml, 读取相应字段,然后使用*`folly::dynamic`*序列化输出json,
*`rapidxml`*使用很简单,秩序要到官网下载对应的几个[头文件](http://rapidxml.sourceforge.net/),加入到对应的库当中,即可
上代码

```cpp
#include "../rapidxml.hpp"
#include "../rapidxml_utils.hpp"
#include <string>
#include <cassert>
#include <iostream>
#include <cstring>
#include <string.h>
using namespace std;
struct OBJ
{
    int xmin, ymin,  xmax, ymax;
};

struct TrafficConePosition
{
    char *filename;
    vector<OBJ> objs;
};
//遍历 xml 兄弟节点
void GetObjs(rapidxml::xml_node<>* root_node, vector<TrafficConePosition> &vcone)
{
    TrafficConePosition tfconep;
    for (rapidxml::xml_node<>* tmp_node = root_node->first_node(); tmp_node; 
    tmp_node = tmp_node->next_sibling())
    {

        if (strcmp(tmp_node->name(), "path") == 0)
            tfconep.filename = tmp_node->value();

        if (strcmp(tmp_node->name(), "object") == 0)
        {
            OBJ obj;
            //进入子节点层
            for (rapidxml::xml_node<>* obj_node = tmp_node->first_node(); obj_node; 
            obj_node = obj_node->next_sibling())
            {
                if (strcmp(obj_node->name(), "bndbox") == 0)
                {
                	#再次进入子节点层
                    rapidxml::xml_node<>* sub_obj_node = obj_node->first_node();
                    if (strcmp(sub_obj_node->name(), "xmin") == 0)
                        obj.xmin = atoi(sub_obj_node->value());

                    sub_obj_node = sub_obj_node->next_sibling();
                    if (strcmp(sub_obj_node->name(), "ymin") == 0)
                        obj.ymin = atoi(sub_obj_node->value());

                    sub_obj_node = sub_obj_node->next_sibling();
                    if (strcmp(sub_obj_node->name(), "xmax") == 0)
                        obj.xmax = atoi(sub_obj_node->value());

                    sub_obj_node = sub_obj_node->next_sibling();
                    if (strcmp(sub_obj_node->name(), "ymax") == 0)
                        obj.ymax = atoi(sub_obj_node->value());
                }
            }
            tfconep.objs.push_back(obj);
        }

    }
    vcone.push_back(tfconep);
}
```
需要指出的是, *`rapidxml`*的设计方法跟普通的认知不同, 通常我们认为*`<node>hello</node>`*是一个*`node`* 其中*`name = node`* *`value = hello`*并且它再没有字节点,但是rapidxml设计当中,该节点的*`first_node()`仍然存在, 只是其*`name = ""`*  *`value = hello`*, 因此在递归遍历的时候,迭代停止条件我找了很半天,并且使用*`name == NULL`*来判断是不起作用的,最后我参考了一个[网上的内容](https://stackoverflow.com/questions/5465227/recursion-problem-in-parsing-with-rapidxml-c-class-pointers-side-effect)实现了一个递归遍历

```cpp
rapidxml::xml_node<>* get_child(rapidxml::xml_node<> *inputNode, string sNodeFilter)
{
    // cycles every child
    for (rapidxml::xml_node<> *nodeChild = inputNode->first_node(); 
    nodeChild; nodeChild = nodeChild->next_sibling())
    {
        if (strlen(nodeChild->name()) != 0)
        {
            cout << "<" << nodeChild->name()<<">" ;
            rapidxml::xml_node<> * x = get_child(nodeChild, sNodeFilter);
            cout<<nodeChild->value();
            if (x)
                return x;

            cout << "</" << nodeChild->name()<<">" << endl;
        }
    }
    return 0;
}
```
>总结一下: 递归函数的几个要素
>1. 明确你这个函数想要干什么
>2. 寻找递归结束条件
>3. 找出函数的等价关系式
>4. 要有返回值


言归正传解析完之后,就需要用folly来对其进行填充

```cpp
int main(int argc, char * argv[])
{
    vector<TrafficConePosition> vcone;


    string input_file = "/home/z/Desktop/TrafficCone/VOC2007/Annotations/";
    vector <string> fPaths, dPaths;


    rapidxml::file<> fdoc("/home/z/Desktop/TrafficCone/VOC2007/Annotations/49.xml");
    rapidxml::xml_document<> doc;
    doc.parse<0>(fdoc.data());
    rapidxml::xml_node<> * root_node = doc.first_node();
    GetObjs(root_node, vcone);


    folly::dynamic cone_json =folly::dynamic::array();

    folly::dynamic value = folly::dynamic::object("_id", "")
                           ("md5", "md5_val")
                           ("filename", "trafficcone/+md5_val")
                           ("ext", "jpg")
                           ("tags", folly::dynamic::array("traffic_cone"))
                           ("others", folly::dynamic::object
                           ("annotator", "zlatan")
                           ("creator","zlatan")
                           ("createdAt", "2019-12-22T09:30:42.922Z")
                           ("updatedAt", "2019-12-22T14:30:42.922Z"));
                           
    folly::dynamic targets =folly::dynamic::array();
    for (size_t i = 0; i < vcone[0].objs.size(); i++)
    {
        folly::dynamic anvalue = folly::dynamic::object
                ("id", "")
                ("name", "traffic_cone")
                ("type", "RECT")
                ("data",folly::dynamic::object
                        ("xmin", vcone[0].objs[i].xmin)
                        ("ymin", vcone[0].objs[i].ymin)
                        ("width", vcone[0].objs[i].xmax-vcone[0].objs[i].xmin)
                        ("height", vcone[0].objs[i].ymax-vcone[0].objs[i].ymin)
                );

        targets.push_back(anvalue);
    }
    value.insert(folly::to<string>("targets"), targets);
    cone_json.push_back(value);

    auto tmpjsonres = folly::toJson(cone_json);
    std::cout<<tmpjsonres<<std::endl;

    return 0;
}
```
过程中,对于要在循环中添加并列的targets, 无从下手,查找资料后知道,可以顶一个类似于vector的变量,将循环中的所有变量push_back到其中即可, *`folly::dynamic targets =folly::dynamic::array();`*
#Readxml
