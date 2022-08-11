#include "rclcpp/rclcpp.hpp"
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/visualization/pcl_visualizer.h>
#include "sensor_msgs/msg/point_cloud2.hpp"

#include "LidarOdometry.hpp"
#include <ctime>

clock_t t0, t1;

class point_cloud_node: public rclcpp::Node
{
    /*
    创建point_cloud节点 
    */
    public:
        std::string viz_name;
        boost::shared_ptr<pcl::visualization::PCLVisualizer> visualizer;
        rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_point_cloud;

        LidarOdometry lidar_odometry;

        point_cloud_node(std::string name): Node(name)
        {
            RCLCPP_INFO(this->get_logger(), "point_cloud_node已创建");

            /*创建并初始化接收*/
            std::string topic_name = "/rslidar_points";
            sub_point_cloud = this->create_subscription<sensor_msgs::msg::PointCloud2>(topic_name, 10, std::bind(&point_cloud_node::callback, this, std::placeholders::_1));
        
            /*配置可视化*/
            viz_name = "pcl cloud";
            visualizer = boost::make_shared<pcl::visualization::PCLVisualizer>(viz_name);
        }

        void callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg_ptr)
        {
            /*处理点云*/
            pcl::PointCloud<pcl::PointXYZI>::Ptr cloud = boost::make_shared<pcl::PointCloud<pcl::PointXYZI>>();
            pcl::fromROSMsg(*msg_ptr, *cloud);

            /*运行算法*/
            t0 = clock();
            lidar_odometry.feature_extraction(*cloud);
            //lidar_odometry.NewtonGussian();
            //*cloud = lidar_odometry.transform(*cloud, lidar_odometry.T);
            t1 = clock();
            double endtime=(double)(t1-t0)/CLOCKS_PER_SEC;
            cout<<"Total time:"<<endtime*1000<<"ms"<<endl;

            /*可视化点云*/
            auto ptr = lidar_odometry.edge_points.makeShared();
            auto ptr2 = cloud;
            //pcl::visualization::PointCloudColorHandlerGenericField<pcl::PointXYZI> intensity(ptr, "intensity");
            pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZI> rgb(ptr2, 155, 155, 155);
            pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZI> intensity(ptr, 255, 0, 0);
	
            visualizer->removeAllPointClouds();
            visualizer->addPointCloud(ptr2, rgb, "2", 0);
            visualizer->addPointCloud(ptr, intensity, viz_name, 0);
            visualizer->spinOnce(0.001);
        }
};


int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<point_cloud_node>("point_cloud_node");

    rclcpp::spin(node);
    rclcpp::shutdown();
    
    return 0;
}
