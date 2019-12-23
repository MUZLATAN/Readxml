#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <string>
#include <iostream>
#include <string.h>
#include <boost/algorithm/hex.hpp>
#include <folly/dynamic.h>
#include <folly/json.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
using namespace std;


rapidxml::xml_node<>* get_child(rapidxml::xml_node<> *inputNode, string sNodeFilter)
{
    // cycles every child
    for (rapidxml::xml_node<> *nodeChild = inputNode->first_node(); nodeChild; nodeChild = nodeChild->next_sibling())
    {
        if (nodeChild->name() == sNodeFilter)
        {
            cout << "node name " << nodeChild->name() << "\n";
            cout << "nodeChild " << nodeChild << endl;
            // returns the desired child
            return nodeChild;
        }
        rapidxml::xml_node<> * x = get_child(nodeChild, sNodeFilter);
        if (x)
            return x;
    }
    return 0;
}
struct OBJ
{
    int xmin, ymin,  xmax, ymax;
};

struct TrafficConePosition
{
    char *filename;
    vector<OBJ> objs;
};
void GetObjs(rapidxml::xml_node<>* root_node, vector<TrafficConePosition> &vcone)
{
    TrafficConePosition tfconep;
    for (rapidxml::xml_node<>* tmp_node = root_node->first_node(); tmp_node; tmp_node = tmp_node->next_sibling())
    {

        if (strcmp(tmp_node->name(), "path") == 0)
            tfconep.filename = tmp_node->value();

        if (strcmp(tmp_node->name(), "object") == 0)
        {
            OBJ obj;
            for (rapidxml::xml_node<>* obj_node = tmp_node->first_node(); obj_node; obj_node = obj_node->next_sibling())
            {
                if (strcmp(obj_node->name(), "bndbox") == 0)
                {
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
//        cout<<vcone[0].objs[i].xmin<<"  "<<vcone[0].objs[i].ymin<<"  "<<vcone[0].objs[i].xmax<<"  "<<vcone[0].objs[i].ymax<<"---";
        targets.push_back(anvalue);
    }
    value.insert(folly::to<string>("targets"), targets);
    cone_json.push_back(value);


    auto tmpjsonres = folly::toJson(cone_json);
    std::cout<<tmpjsonres<<std::endl;

    return 0;
}
