/****************************************************************************************
 *
 * Copyright (c) 2024, Shenyang Institute of Automation, Chinese Academy of Sciences
 *
 * Authors: Yanpeng Jia
 * Contact: jiayanpeng@sia.cn
 *
 ****************************************************************************************/

#include "trlo/trlo.h"

class trlo::OdomNode {

public:

  OdomNode(ros::NodeHandle node_handle);
  ~OdomNode();

  static void abort() {
    abort_ = true;
  }

  void start();
  void stop();

private:

  void abortTimerCB(const ros::TimerEvent& e);
  void icpCB(const sensor_msgs::PointCloud2ConstPtr& pc);
  void imuCB(const sensor_msgs::Imu::ConstPtr& imu);
  void boxCB(const jsk_recognition_msgs::BoundingBoxArrayPtr& box);
  bool saveTrajectory(trlo::save_traj::Request& req,
                      trlo::save_traj::Response& res);

  void getParams();
  void InitParam();
  void allocateMemory();

  void publishToROS();
  void publishPose();
  void publishTrajectory();
  void publishTransform();
  void publishKeyframe();
  void publishRobot();

  void preprocessPoints();
  void removeClosedPointCloud(const pcl::PointCloud<pcl::PointXYZI> &cloud_in, pcl::PointCloud<pcl::PointXYZI> &cloud_out, float th1, float th2);
  void initializeInputTarget();
  void setInputSources();

  void initializeTRLO();
  void gravityAlign();

  void getNextPose();
  void integrateIMU();

  void propagateS2S(Eigen::Matrix4f T);
  void propagateS2M();

  void setAdaptiveParams();

  void computeMetrics();
  void computeSpaciousness();

  void transformCurrentScan();
  void updateKeyframes();
  void computeConvexHull();
  void computeConcaveHull();
  void pushSubmapIndices(std::vector<float> dists, int k, std::vector<int> frames);
  void getSubmapKeyframes();

  void debug();

  double first_imu_time;

  ros::NodeHandle nh;
  ros::Timer abort_timer;
  
  ros::ServiceServer save_traj_srv;

  ros::Subscriber icp_sub;
  ros::Subscriber imu_sub;
  ros::Subscriber box_sub;

  ros::Publisher odom_pub;
  ros::Publisher trajectory_pub;
  ros::Publisher pose_pub;
  ros::Publisher keyframe_pub;
  ros::Publisher kf_pub;
  ros::Publisher robot_pub;

  Eigen::Vector3f origin;
  std::vector<std::pair<Eigen::Vector3f, Eigen::Quaternionf>> trajectory;
  std::vector<std::pair<std::pair<Eigen::Vector3f, Eigen::Quaternionf>, pcl::PointCloud<PointType>::Ptr>> keyframes;
  std::vector<std::vector<Eigen::Matrix4d, Eigen::aligned_allocator<Eigen::Matrix4d>>> keyframe_normals;

  std::atomic<bool> trlo_initialized;
  std::atomic<bool> imu_calibrated;

  std::string odom_frame;
  std::string child_frame;

  pcl::PointCloud<PointType>::Ptr current_scan;
  pcl::PointCloud<PointType>::Ptr current_scan_t;

  pcl::PointCloud<PointType>::Ptr keyframes_cloud;
  pcl::PointCloud<PointType>::Ptr keyframe_cloud;
  int num_keyframes;

  pcl::ConvexHull<PointType> convex_hull;
  pcl::ConcaveHull<PointType> concave_hull;
  std::vector<int> keyframe_convex;
  std::vector<int> keyframe_concave;

  pcl::PointCloud<PointType>::Ptr submap_cloud;
  std::vector<Eigen::Matrix4d, Eigen::aligned_allocator<Eigen::Matrix4d>> submap_normals;

  std::vector<int> submap_kf_idx_curr;
  std::vector<int> submap_kf_idx_prev;
  std::atomic<bool> submap_hasChanged;
  std::unordered_set<int> submap_kf_idx_hash;

  pcl::PointCloud<PointType>::Ptr source_cloud;
  pcl::PointCloud<PointType>::Ptr target_cloud;

  ros::Time scan_stamp;

  double curr_frame_stamp;
  double prev_frame_stamp;
  std::vector<double> comp_times;
  std::vector<double> submap_build_times;
  std::vector<double> ground_optimize_times;

  nano_gicp::NanoGICP<PointType, PointType> gicp_s2s;
  nano_gicp::NanoGICP<PointType, PointType> gicp;

  pcl::CropBox<PointType> crop;
  pcl::VoxelGrid<PointType> vf_scan;
  pcl::VoxelGrid<PointType> vf_submap;

  nav_msgs::Odometry odom;
  nav_msgs::Odometry kf;

  geometry_msgs::PoseStamped pose_ros;

  Eigen::Matrix4f T, T_S2S_pre;
  Eigen::Matrix4f T_s2s, T_s2s_prev;

  Eigen::Vector3f pose_s2s;
  Eigen::Matrix3f rotSO3_s2s;
  Eigen::Quaternionf rotq_s2s;

  Eigen::Vector3f pose;
  Eigen::Matrix3f rotSO3;
  Eigen::Quaternionf rotq;

  Eigen::Matrix4f imu_SE3;

  struct XYZd {
    double x;
    double y;
    double z;
  };

  struct ImuBias {
    XYZd gyro;
    XYZd accel;
  };

  ImuBias imu_bias;

  struct ImuMeas {
    double stamp;
    XYZd ang_vel;
    XYZd lin_accel;
  };

  ImuMeas imu_meas;

  boost::circular_buffer<ImuMeas> imu_buffer;
  boost::circular_buffer<jsk_recognition_msgs::BoundingBoxArray> box_buffer;

  static bool comparatorImu(ImuMeas m1, ImuMeas m2) {
    return (m1.stamp < m2.stamp);
  };

  struct Metrics {
    std::vector<float> spaciousness;
  };

  Metrics metrics;

  static std::atomic<bool> abort_;
  std::atomic<bool> stop_publish_thread;
  std::atomic<bool> stop_publish_keyframe_thread;
  std::atomic<bool> stop_metrics_thread;
  std::atomic<bool> stop_debug_thread;

  std::thread publish_thread;
  std::thread publish_keyframe_thread;
  std::thread metrics_thread;
  std::thread debug_thread;

  std::mutex mtx_imu;
  std::mutex mtx_box;

  std::string cpu_type;
  std::vector<double> cpu_percents;
  clock_t lastCPU, lastSysCPU, lastUserCPU;
  int numProcessors;

  // Parameters
  std::string version_;

  bool gravity_align_;

  double keyframe_thresh_dist_;
  double keyframe_thresh_rot_;

  int submap_knn_;
  int submap_kcv_;
  int submap_kcc_;
  double submap_concave_alpha_;

  bool initial_pose_use_;
  Eigen::Vector3f initial_position_;
  Eigen::Quaternionf initial_orientation_;

  bool crop_use_;
  double crop_size_;

  bool vf_scan_use_;
  double vf_scan_res_;

  bool vf_submap_use_;
  double vf_submap_res_;

  bool adaptive_params_use_;

  bool imu_use_;
  int imu_calib_time_;
  int imu_buffer_size_;

  int box_buffer_size_;

  int gicp_min_num_points_;

  int gicps2s_k_correspondences_;
  double gicps2s_max_corr_dist_;
  int gicps2s_max_iter_;
  double gicps2s_transformation_ep_;
  double gicps2s_euclidean_fitness_ep_;
  int gicps2s_ransac_iter_;
  double gicps2s_ransac_inlier_thresh_;

  int gicps2m_k_correspondences_;
  double gicps2m_max_corr_dist_;
  int gicps2m_max_iter_;
  double gicps2m_transformation_ep_;
  double gicps2m_euclidean_fitness_ep_;
  int gicps2m_ransac_iter_;
  double gicps2m_ransac_inlier_thresh_;
  
  nav_msgs::Path robot_trajectory;

  bool ground_use_;
  double para_tz[1] = {0};
  double para_tz_pre = 0;
  double para_q[4] = {0, 0, 0, 1};

  Eigen::Vector3f ground_normal;
  double ground_threshold_;
};
