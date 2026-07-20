#include <rclcpp/rclcpp.hpp>
#include "hbm_img_msgs/msg/hbm_msg1080_p.hpp"
#include <opencv2/opencv.hpp>
#include <zbar.h>
#include <std_msgs/msg/int32.hpp> // 1. 修改包含头文件

class Qrcode : public rclcpp::Node
{
public:
  Qrcode() : Node("qrcode"), number_i_(0)
  {
    rclcpp::QoS qos(1);
    qos.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
    subscriber_hbmem_ = this->create_subscription<hbm_img_msgs::msg::HbmMsg1080P>(
        "/nv12_img", qos, std::bind(&Qrcode::subscription_callback, this, std::placeholders::_1));
        
    // 2. 修改发布器类型为 Int32
    qrcode_number_publisher_ = this->create_publisher<std_msgs::msg::Int32>("/qrcode_number", 10);
  }

private:
  void subscription_callback(const hbm_img_msgs::msg::HbmMsg1080P::SharedPtr msg)
  {
    if (!msg)
      return;
    // number_i_ += 1;
    // number_i_ = number_i_ % 2;
    // if (number_i_ != 0)
    //   return;
    
    int height = msg->height;
    int width = msg->width;
    size_t step = msg->step;
    cv::Mat y_plane(height, width, CV_8UC1, msg->data.data(), step);
    cv::Mat gray = y_plane;
    
    zbar::ImageScanner scanner;
    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
    zbar::Image zbar_image(width, height, "Y800", gray.data, width * height);
    int result = scanner.scan(zbar_image);
    
    if (result > 0)
    {
      for (zbar::Image::SymbolIterator symbol = zbar_image.symbol_begin(); symbol != zbar_image.symbol_end(); ++symbol)
      {
        std::string qr_data = symbol->get_data();
        
        // 只需要声明你要发布的 Int32 消息即可
        std_msgs::msg::Int32 qrcode_number_msg;

        if (qr_data == "ClockWise") // 顺时针
        {
          qrcode_number_msg.data = 3; 
        }
        else if (qr_data == "AntiClockWise") // 逆时针
        {
          qrcode_number_msg.data = 4; 
        }
        else
        {
          try
          {
            int number = std::stoi(qr_data);
            if (number >= 1 && number <= 9999)
            {
              // 直接把算好的结果塞给要发布的变量
              qrcode_number_msg.data = (number % 2 == 0) ? 4 : 3;
            }
            else
            {
              RCLCPP_WARN(this->get_logger(), "Recognized number out of range (1-9999): %d", number);
              continue;
            }
          }
          catch (const std::invalid_argument &e)
          {
            RCLCPP_WARN(this->get_logger(), "Unrecognized content: %s", qr_data.c_str());
            continue;
          }
        }

        // 统一在这里发布
        qrcode_number_publisher_->publish(qrcode_number_msg);
      }
    }
  }
  
  rclcpp::Subscription<hbm_img_msgs::msg::HbmMsg1080P>::SharedPtr subscriber_hbmem_;
  // 5. 修改类成员变量类型为 Int32
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr qrcode_number_publisher_;
  int number_i_ = 0;
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Qrcode>());
  rclcpp::shutdown();
  return 0;
}











// #include <rclcpp/rclcpp.hpp>
// #include "hbm_img_msgs/msg/hbm_msg1080_p.hpp"
// #include <opencv2/opencv.hpp>
// #include <zbar.h>
// #include <std_msgs/msg/string.hpp>
// #include "origincar_msg/msg/sign.hpp"
// class Qrcode : public rclcpp::Node
// {
// public:
//   Qrcode() : Node("qrcode"), number_i_(0)
//   {
//     rclcpp::QoS qos(1);
//     qos.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
//     subscriber_hbmem_ = this->create_subscription<hbm_img_msgs::msg::HbmMsg1080P>(
//         "/nv12_img", qos, std::bind(&Qrcode::subscription_callback, this, std::placeholders::_1));
//     qrcode_number_publisher_ = this->create_publisher<std_msgs::msg::String>("/qrcode_number", 10);
//   }

// private:
//   void subscription_callback(const hbm_img_msgs::msg::HbmMsg1080P::SharedPtr msg)
//   {
//     if (!msg)
//       return;
//     // number_i_ += 1;
//     // number_i_ = number_i_ % 2;
//     // if (number_i_ != 0)
//     //   return;
//     int height = msg->height;
//     int width = msg->width;
//     size_t step = msg->step;
//     cv::Mat y_plane(height, width, CV_8UC1, msg->data.data(), step);
//     cv::Mat gray = y_plane;
//     zbar::ImageScanner scanner;
//     scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
//     zbar::Image zbar_image(width, height, "Y800", gray.data, width * height);
//     int result = scanner.scan(zbar_image);
//     if (result > 0)
//     {
//       for (zbar::Image::SymbolIterator symbol = zbar_image.symbol_begin(); symbol != zbar_image.symbol_end(); ++symbol)
//       {
//         std::string qr_data = symbol->get_data();
//         std_msgs::msg::String qrcode_number_msg;
//         origincar_msg::msg::Sign sign_msg;
//         if (qr_data == "ClockWise") // 顺时针
//         {
//           sign_msg.sign_data = 3;
//           qrcode_number_msg.data = qr_data;
//         }
//         else if (qr_data == "AntiClockWise") // 逆时针
//         {
//           sign_msg.sign_data = 4;
//           qrcode_number_msg.data = qr_data;
//         }
//         else
//         {
//           try
//           {
//             int number = std::stoi(qr_data);
//             if (number >= 1 && number <= 9999)
//             {
//               sign_msg.sign_data = (number % 2 == 0) ? 4 : 3;
//               qrcode_number_msg.data = qr_data;
//             }
//             else
//             {
//               RCLCPP_WARN(this->get_logger(), "Recognized number out of range (1-9999): %d", number);
//               continue;
//             }
//           }
//           catch (const std::invalid_argument &e)
//           {
//             RCLCPP_WARN(this->get_logger(), "Unrecognized content: %s", qr_data.c_str());
//             continue;
//           }
//         }
//         qrcode_number_publisher_->publish(qrcode_number_msg);
//       }
//     }
//   }
//   rclcpp::Subscription<hbm_img_msgs::msg::HbmMsg1080P>::SharedPtr subscriber_hbmem_;
//   rclcpp::Publisher<std_msgs::msg::String>::SharedPtr qrcode_number_publisher_;
//   int number_i_ = 0;
// };
// int main(int argc, char *argv[])
// {
//   rclcpp::init(argc, argv);
//   rclcpp::spin(std::make_shared<Qrcode>());
//   rclcpp::shutdown();
//   return 0;
// }