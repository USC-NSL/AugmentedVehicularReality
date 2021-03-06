//
// Created by hang on 3/11/17.
//

#include "pcCodec.hpp"

pcCodec::pcCodec(int width, int height) : width(width), height(height) {
//    data_cloud = new float[height * width * 4];
    compressionProfile = pcl::io::MED_RES_ONLINE_COMPRESSION_WITH_COLOR;

    PointCloudEncoder = new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA> (compressionProfile, false);
    PointCloudDecoder = new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA> ();

    point_cloud_xyzrgba_ptr = pcl::PointCloud<pcl::PointXYZRGBA>::Ptr(new pcl::PointCloud<pcl::PointXYZRGBA>);
    point_cloud_xyz_ptr = pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>);
    point_cloud_xyz_ptr_lowRes = pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>);

//    point_cloud_xyzrgba_ptr->points.resize(height*width/15);
    point_cloud_xyzrgba_ptr->points.resize(height*width);
    point_cloud_xyz_ptr->points.resize(height*width);
    if (RX){
        point_cloud_xyz_ptr_lowRes->points.resize(height/resRatio*width*2/resRatio);
    }else{
        point_cloud_xyz_ptr_lowRes->points.resize(height/resRatio*width/resRatio);
    }

    planeSeg_coefficients_ptr =  pcl::ModelCoefficients::Ptr(new pcl::ModelCoefficients);
    planeSeg_inliers_ptr= pcl::PointIndices::Ptr(new pcl::PointIndices);
}

pcCodec::~pcCodec() {
//    delete[] data_cloud;
    delete (PointCloudEncoder);
    delete (PointCloudDecoder);
//    delete (point_cloud_rgba_ptr);
}


//cv::Mat pcCodec::pcl2cv_xyz(pcl::PointXYZ &pc_xyz) {
//
//    cv::Mat ret;
//    ret = cv::Mat( pc_xyz.height, pc_xyz->width, CV_8UC4 );
//    if( pc_xyz.points.empty() ){
//
//
//
//// pcl::PointCloud to cv::Mat
//#pragma omp parallel for
//        for( int y = 0; y < image.rows; y++ ) {
//            for( int x = 0; x < image.cols; x++ ) {
//                pcl::PointXYZRGBA point = cloud->at( x, y );
//                image.at<cv::Vec4b>( y, x )[0] = point.b;
//                image.at<cv::Vec4b>( y, x )[1] = point.g;
//                image.at<cv::Vec4b>( y, x )[2] = point.r;
//                image.at<cv::Vec4b>( y, x )[3] = point.a;
//            }
//        }
//    }
//}

bool pcCodec::cv2pcl_xyz(cv::Mat &pc){


    if (pc.empty()){
        cout << "empty pc\n";
        return false;
    }
    if (pc.cols != width || pc.rows != height) {
        cerr << "Point cloud compression: matrix size doesn't match\n";
        return false;
    }

    float* p_cloud;
    sl::Mat slpc;
    cvMat2slMat(pc,slpc, sl::MEM_CPU);
    p_cloud = slpc.getPtr<float>(sl::MEM_CPU); // Get the pointer
    // Fill the buffer
//    mutex_input.lock(); // To prevent from data corruption
//    memcpy(data_cloud, p_cloud, width * height * sizeof (float) * 4);
//    mutex_input.unlock();

//    int index4 = height*width/2;
    int index4 = 0;

#ifdef EVAL
    timeval start,end;
    gettimeofday(&start,NULL);
#endif
    // for rbga
    for (auto &it : point_cloud_xyz_ptr->points) {
        float X = p_cloud[index4 * 4];
        if (!isValidMeasure(X)) // Checking if it's a valid point
            it.x = it.y = it.z = 0;
        else {
            it.x = X;
            it.y = p_cloud[index4 * 4 + 1];
            it.z = p_cloud[index4 * 4 + 2];
//            it.rgb = data_cloud[index4 * 4 + 3];
        }
        index4++;
    }
#ifdef EVAL
    gettimeofday(&end,NULL);
    cout << "Total conversion time: " << double(end.tv_sec-start.tv_sec)*1000 + double(end.tv_usec-start.tv_usec) / 1000 << "ms" << endl;
#endif
//    cout << "finished conversion\n";
    slpc.free(MEM_CPU);
    return true;
}


bool pcCodec::cv2pcl_xyz_lowRes(cv::Mat &pc){


    if (pc.empty()){
        cout << "empty pc\n";
        return false;
    }


//    if (pc.cols != width || pc.rows != height) {
//        cerr << "Point cloud compression: matrix size doesn't match\n";
//        return false;
//    }

    float* p_cloud;
    sl::Mat slpc;
    cvMat2slMat(pc,slpc, sl::MEM_CPU);
    p_cloud = slpc.getPtr<float>(sl::MEM_CPU); // Get the pointer
    // Fill the buffer
//    mutex_input.lock(); // To prevent from data corruption
//    memcpy(data_cloud, p_cloud, width * height * sizeof (float) * 4);
//    mutex_input.unlock();

//    int index4 = height*width/2;
    int index4 = 0;

#ifdef EVAL
    timeval start,end;
    gettimeofday(&start,NULL);
#endif
    // for rbga
    for (auto &it : point_cloud_xyz_ptr_lowRes->points) {
        int col = index4 % (pc.cols/resRatio);
        int row = index4 / (pc.cols/resRatio);
        int index = (row*(pc.cols)+col)*resRatio*4;
        float X = p_cloud[index];
        if (!isValidMeasure(X)) // Checking if it's a valid point
            it.x = it.y = it.z = 0;
        else {
            it.x = X;
            it.y = p_cloud[index + 1];
            it.z = p_cloud[index + 2];
//            it.rgb = data_cloud[index4 * 4 + 3];
        }
        index4++;
    }
#ifdef EVAL
    gettimeofday(&end,NULL);
    cout << "Total conversion time: " << double(end.tv_sec-start.tv_sec)*1000 + double(end.tv_usec-start.tv_usec) / 1000 << "ms" << endl;
#endif
//    cout << "finished conversion\n";
    slpc.free(MEM_CPU);
    return true;
}

bool pcCodec::cv2pcl_xyzrgba(cv::Mat &pc){


    if (pc.empty()){
        cout << "empty pc\n";
        return false;
    }
    if (pc.cols != width || pc.rows != height) {
        cerr << "Point cloud compression: matrix size doesn't match\n";
        return false;
    }

    float* p_cloud;
    sl::Mat slpc;
    cvMat2slMat(pc,slpc,sl::MEM_CPU);
    p_cloud = slpc.getPtr<float>(sl::MEM_CPU); // Get the pointer
    // Fill the buffer
//    mutex_input.lock(); // To prevent from data corruption
//    memcpy(p_cloud, p_cloud, width * height * sizeof (float) * 4);
//    mutex_input.unlock();

//    int index4 = height*width/2;
    int index4 = 0;

#ifdef EVAL
    timeval start,end;
    gettimeofday(&start,NULL);
#endif
    // for rbga
    for (auto &it : point_cloud_xyzrgba_ptr->points) {
        float X = p_cloud[index4 * 4];
        if (!isValidMeasure(X)) // Checking if it's a valid point
            it.x = it.y = it.z = it.rgb = 0;
        else {
            it.x = X;
            it.y = p_cloud[index4 * 4 + 1];
            it.z = p_cloud[index4 * 4 + 2];
            it.rgb = p_cloud[index4 * 4 + 3];
        }
        index4++;
    }
#ifdef EVAL
    gettimeofday(&end,NULL);
    cout << "Total conversion time: " << double(end.tv_sec-start.tv_sec)*1000 + double(end.tv_usec-start.tv_usec) / 1000 << "ms" << endl;
#endif
//    cout << "finished conversion\n";
    slpc.free(MEM_CPU);
    return true;
}

void pcCodec::encode(cv::Mat& pc){

    if (!cv2pcl_xyzrgba(pc)) return;

#ifdef EVAL
    timeval start,end;
    gettimeofday(&start,NULL);
    cout << "TimeStamp: " << double(start.tv_sec-tInit.tv_sec)*1000 + double(start.tv_usec-tInit.tv_usec) / 1000 << "ms: ";
    cout << "encoding start\n";
#endif
    std::stringstream compressedData;
    // compress point cloud
    PointCloudEncoder->encodePointCloud(point_cloud_xyzrgba_ptr, compressedData);

#ifdef EVAL
    gettimeofday(&end,NULL);
    cout << "TimeStamp: " << double(end.tv_sec-tInit.tv_sec)*1000 + double(end.tv_usec-tInit.tv_usec) / 1000 << "ms: ";
    cout << "Encoding time: " << double(end.tv_sec-start.tv_sec)*1000 + double(end.tv_usec-start.tv_usec) / 1000 << "ms" << endl;
#endif
    if (DEBUG){

        compressedData.seekp(0, ios::end);
        std::stringstream::pos_type offset = compressedData.tellp();
        cout << "compressed size:" << offset << endl;
    }
//    return compressedData;
}



void pcCodec::planeSegmentation(cv::Mat &pc, sl::Mat & slpc){
//    if (!cv2pcl_xyz(pc)) return;

    if (!cv2pcl_xyz_lowRes(pc)) return;

    // Create the segmentation object
    pcl::SACSegmentation<pcl::PointXYZ> seg;
    // Optional
    seg.setOptimizeCoefficients (true);
    // Mandatory
    seg.setModelType (pcl::SACMODEL_PLANE);
    seg.setMethodType (pcl::SAC_RANSAC);
//    seg.setMaxIterations(1000);
    seg.setDistanceThreshold (0.01);

    seg.setInputCloud (point_cloud_xyz_ptr_lowRes );
    seg.segment (*planeSeg_inliers_ptr, *planeSeg_coefficients_ptr);

    if (planeSeg_inliers_ptr->indices.size () == 0)
    {
        PCL_ERROR ("Could not estimate a planar model for the given dataset.");
        return;
    }

    std::cerr << "Model coefficients: " << planeSeg_coefficients_ptr->values[0] << " "
              << planeSeg_coefficients_ptr->values[1] << " "
              << planeSeg_coefficients_ptr->values[2] << " "
              << planeSeg_coefficients_ptr->values[3] << std::endl;

    std::cerr << "Model inliers: " << planeSeg_inliers_ptr->indices.size () << std::endl;

    /// visual debug
//    sl::Mat slpc;
    cvMat2slMat(pc,slpc,sl::MEM_CPU);
    float * p_cloud = slpc.getPtr<float>(sl::MEM_CPU); // Get the pointer
    /// color all low res points
    int index4 = 0;
    for (auto &it : point_cloud_xyz_ptr_lowRes->points){
        int col = index4 % (pc.cols/resRatio);
        int row = index4 / (pc.cols/resRatio);
        int index = (row*(pc.cols)+col)*resRatio*4;
        p_cloud[index + 3] = 0xFFFFFFFF;
        index4++;
    }


    /// color plane inliers
    for (size_t i=0;i<planeSeg_inliers_ptr->indices.size();i++){
        int col = planeSeg_inliers_ptr->indices[i] % (pc.cols/resRatio);
        int row = planeSeg_inliers_ptr->indices[i] / (pc.cols/resRatio);
        int index = (row*(pc.cols)+col)*resRatio*4;
        /// color plane inliers
        p_cloud[index + 3] = 0xFFFF00FF;
        /// color plane
        for (int j=1;j<resRatio-1;j++){
            for (int k=1;k<resRatio-1;k++){
                int idx = ((row*resRatio+j)*pc.cols+col*resRatio+k)*4;
                p_cloud[idx + 3] = 0xFF0000FF;
            }
        }
    }

}
/// manually segment based on ax+by+cz+d=0
void pcCodec::planeSegmentation_ManualPlaneModel(cv::Mat &pc, sl::Mat & slpc, float A, float B, float C, float D){
//    if (!cv2pcl_xyz(pc)) return;

    if (!cv2pcl_xyz_lowRes(pc)) return;
    cout << "Model Coeeffiencts: " << A << " "<< B << " "<< C << " " << D << endl;
    /// visual debug
//    sl::Mat slpc;
    cvMat2slMat(pc,slpc,sl::MEM_CPU);
    float * p_cloud = slpc.getPtr<float>(sl::MEM_CPU); // Get the pointer
    /// color all low res points
    int index4 = 0;
    for (auto &it : point_cloud_xyz_ptr_lowRes->points){
        int col = index4 % (pc.cols/resRatio);
        int row = index4 / (pc.cols/resRatio);
        int index = (row*(pc.cols)+col)*resRatio*4;
        p_cloud[index + 3] = 0xFFFFFFFF;
        index4++;
    }


    /// color plane inliers
    for (size_t i=0;i<point_cloud_xyz_ptr_lowRes->points.size();i++){
        int col = i % (pc.cols/resRatio);
        int row = i / (pc.cols/resRatio);
        int index = (row*(pc.cols)+col)*resRatio*4;
        float x = p_cloud[index];
        float y = p_cloud[index+1];
        float z = p_cloud[index+2];
        cout << "Point " << i << ": " << "(" << row << "," << col <<  "): " << x << ", " << y << ", " << z;
        if (A*x+B*y+C*z+D<0){
            cout << "inlier" << endl;
            /// color plane inliers
            p_cloud[index + 3] = 0xFFFF00FF;
//            inliersIdx.push_back(index);
            /// color plane
            for (int j=1;j<resRatio-1;j++){
                for (int k=1;k<resRatio-1;k++){
                    int idx = ((row*resRatio+j)*pc.cols+col*resRatio+k)*4;
                    p_cloud[idx + 3] = 0xFF0000FF;
                }
            }
        }else{
            cout << "outlier" << endl;
        }
    }

}


//void pcCodec::euclideanSegmentation(cv::Mat &pc){
////    // Read in the cloud data
////    pcl::PCDReader reader;
////    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>), cloud_f (new pcl::PointCloud<pcl::PointXYZ>);
////    reader.read ("table_scene_lms400.pcd", *cloud);
//
//    if (!cv2pcl_xyz(pc)) return;
//
//    std::cout << "PointCloud before filtering has: " << point_cloud_xyz_ptr->points.size () << " data points." << std::endl; //*
//
//    // Create the filtering object: downsample the dataset using a leaf size of 1cm
//    pcl::VoxelGrid<pcl::PointXYZ> vg;
//    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_f (new pcl::PointCloud<pcl::PointXYZ>);
//    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZ>);
//
//    // down sampling
//    vg.setInputCloud (point_cloud_xyz_ptr);
//    vg.setLeafSize (0.01f, 0.01f, 0.01f);
//    vg.filter (*cloud_filtered);
//    std::cout << "PointCloud after filtering has: " << cloud_filtered->points.size ()  << " data points." << std::endl; //*
//
//    // Create the segmentation object for the planar model and set all the parameters
//    pcl::SACSegmentation<pcl::PointXYZ> seg;
//    pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
//    pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
//    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_plane (new pcl::PointCloud<pcl::PointXYZ> ());
////    pcl::PCDWriter writer;
//    seg.setOptimizeCoefficients (true);
//    seg.setModelType (pcl::SACMODEL_PLANE);
//    seg.setMethodType (pcl::SAC_RANSAC);
//    seg.setMaxIterations (100);
//    seg.setDistanceThreshold (0.02);
//
//    int i=0, nr_points = (int) cloud_filtered->points.size ();
//    while (cloud_filtered->points.size () > 0.3 * nr_points)
//    {
//        // Segment the largest planar component from the remaining cloud
//        seg.setInputCloud (cloud_filtered);
//        seg.segment (*inliers, *coefficients);
//        if (inliers->indices.size () == 0)
//        {
//            std::cout << "Could not estimate a planar model for the given dataset." << std::endl;
//            break;
//        }
//
//        // Extract the planar inliers from the input cloud
//        pcl::ExtractIndices<pcl::PointXYZ> extract;
//        extract.setInputCloud (cloud_filtered);
//        extract.setIndices (inliers);
//        extract.setNegative (false);
//
//        // Get the points associated with the planar surface
//        extract.filter (*cloud_plane);
//        std::cout << "PointCloud representing the planar component: " << cloud_plane->points.size () << " data points." << std::endl;
//
//        // Remove the planar inliers, extract the rest
//        extract.setNegative (true);
//        extract.filter (*cloud_f);
//        *cloud_filtered = *cloud_f;
//    }
//
//    // Creating the KdTree object for the search method of the extraction
//    pcl::search::KdTree<pcl::PointXYZ>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZ>);
//    tree->setInputCloud (cloud_filtered);
//
//    std::vector<pcl::PointIndices> cluster_indices;
//    pcl::EuclideanClusterExtraction<pcl::PointXYZ> ec;
//    ec.setClusterTolerance (0.02); // 2cm
//    ec.setMinClusterSize (100);
//    ec.setMaxClusterSize (25000);
//    ec.setSearchMethod (tree);
//    ec.setInputCloud (cloud_filtered);
//    ec.extract (cluster_indices);
//
//    int j = 0;
//    for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin (); it != cluster_indices.end (); ++it)
//    {
//        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_cluster (new pcl::PointCloud<pcl::PointXYZ>);
//        for (std::vector<int>::const_iterator pit = it->indices.begin (); pit != it->indices.end (); ++pit)
//            cloud_cluster->points.push_back (cloud_filtered->points[*pit]); //*
//        cloud_cluster->width = cloud_cluster->points.size ();
//        cloud_cluster->height = 1;
//        cloud_cluster->is_dense = true;
//
//        std::cout << "PointCloud representing the Cluster: " << cloud_cluster->points.size () << " data points." << std::endl;
//        std::stringstream ss;
//        ss << "cloud_cluster_" << j << ".pcd";
////        writer.write<pcl::PointXYZ> (ss.str (), *cloud_cluster, false); //*
//
//
//
//        j++;
//    }
//}