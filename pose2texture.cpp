#include <getopt.h>
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <algorithm>
#include <vector>
using namespace std;
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
using namespace cv;

//https://stackoverflow.com/questions/24716932/obtaining-list-of-unique-pixel-values-in-opencv-mat
//
std::vector<unsigned char> unique(const cv::Mat& input, bool sort = false)
{
    if (input.channels() > 1 || input.type() != CV_8UC1) {
        std::cerr << "unique !!! Only works with CV_8U 1-channel Mat" << std::endl;
        return std::vector<unsigned char>();
    }

    std::vector<unsigned char> out;
    for (int y = 0; y < input.rows; ++y) {
        const unsigned char* row_ptr = input.ptr<unsigned char>(y);
        for (int x = 0; x < input.cols; ++x) {
            unsigned char value = row_ptr[x];

            if ( std::find(out.begin(), out.end(), value) == out.end() )
                out.push_back(value);
        }
    }

    if (sort)
        std::sort(out.begin(), out.end());

    return out;
}

static struct option long_options[] = {
    {"verbose",  		required_argument, 0, 'v'},
    {"help",     		no_argument,       0, 'h'},
    {"vui-folder",  	required_argument, 0, 'i'},
    {0, 0, 0, 0}
};

int main(int argc, char** argv )
{
    boost::filesystem::path vuipath;

    int 	option_index = 0;
    while(1) {
        int c = getopt_long (argc, argv, "hv:i:",long_options, &option_index);
        if (c == -1)
            break;  // end of options
        switch (c) {
        case 'v':
            //verbose_level=atoi(optarg);
            break;
        case 'h':
            //usage(cerr,basename(argv[0]));
            //exit(1);
            break;		// never reached
        case 'i':
            // path to _INDS.png and _IUV.png folder
            vuipath=boost::filesystem::path(optarg);
            break;
        }
    }


    cout << "vui-folder: " << vuipath << endl;
    Mat   cumul(Size(800,1200),CV_32FC4,Scalar(0,0,0,0));
    int count=0;

    for(int i=optind; i<argc; i++) {
        boost::filesystem::path bgrimpath(argv[i]);
        boost::filesystem::path bgrimbasename=basename(bgrimpath);
        // skip *_INDS.png, *._IUV.png
        if(string::npos!=bgrimbasename.string().find("_INDS")) {
            cerr << basename(argv[0]) << ": skipping _INDS file  '" << argv[i] << "'" << endl;
            continue;
        }
        if(string::npos!=bgrimbasename.string().find("_IUV")) {
            cerr << basename(argv[0]) << ": skipping _IUV file  '" << argv[i] << "'" << endl;
            continue;
        }

        // read the source image file
        Mat imgsrc=imread(argv[i],1);
        if(imgsrc.empty()) {
            cerr << basename(argv[0]) << ": could not open file or invalid image file  '" << argv[i] << "'" << endl;
            continue;
        }
        if(imgsrc.channels()!=3) {
            cerr << basename(argv[0]) << ": image file is not RGB '" << argv[i] << "'" << endl;
            continue;
        }

        Size srcsize=imgsrc.size();

        cout << "processing: " << string(argv[i]) << endl;

        // get IUV file
        boost::filesystem::path vuiimpath= vuipath / boost::filesystem::path(bgrimbasename.string() + string("_IUV.png"));
        cout << "\t- " << vuiimpath << endl;

        Mat vuisrc=imread(vuiimpath.c_str(),1);
        if(vuisrc.empty()) {
            cerr << basename(argv[0]) << ": could not open file or invalid image file  '" << vuiimpath << "'" << endl;
            continue;
        }
        if(vuisrc.channels()!=3) {
            cerr << basename(argv[0]) << ": image file is not RGB '" << vuiimpath << "'" << endl;
            continue;
        }
        Size vuisize=vuisrc.size();
        /*
        // get INDS file
        boost::filesystem::path indimpath= vuipath / boost::filesystem::path(bgrimbasename.string() + string("_INDS.png"));
        cout << "\t- " << indimpath << endl;

        Mat indsrc=imread(indimpath.c_str(),-1);
        if(indsrc.empty()) {
            cerr << basename(argv[0]) << ": could not open file or invalid image file  '" << indimpath << "'" << endl;
            continue;
        }
        if(indsrc.channels()!=1) {
            cerr << basename(argv[0]) << ": image file is not 1 channel'" << indimpath << "'" << endl;
            continue;
        }
        Size indsize=indsrc.size();

        // get number of different values in ind matrix => should be nb of people seen
        std::vector<unsigned char> unik = unique(indsrc, true);

        for (unsigned int i = 0; i < unik.size(); i++)
            std::cout << (short)unik[i] << " ";
        */
        boost::filesystem::path teximpath= boost::filesystem::path(bgrimbasename.string() + string("_TEX.png"));
        cout << "\t- " << teximpath << endl;



        Mat   texture(Size(800,1200),CV_32FC4,Scalar(0,0,0,0));
        //Mat   texture=imread("SURREAL.png",1);
        //4x6=24 tiles, tile size 200*200
        vector<Mat>  tiles;
        for(int row=0; row<4; row++)
            for(int col=0; col<6; col++)
                tiles.push_back(texture(Rect(200*row,200*col,200,200)));


        //Mat   out=imgsrc;
        //out=Scalar(0,0,0);
        for(int row=0; row<vuisize.height; row++)
            for(int col=0; col<vuisize.width; col++) {
                unsigned int ind=0;
                Vec3b        vui=vuisrc.at<Vec3b>(row,col);

                int i= (unsigned)0x1F & vui[0];
                int u=(255.-float(vui[2]))*199./255.;
                int v=float(vui[1])*199./255.;
                /*
                if((i>0)&&(i<25))
                    if((u>=0)&&(u<200))
                        if((v>=0)&&(v<200))
                out.at<Vec3b>(row,col)=tiles[i-1].at<Vec3b>(u,v);
                */
                if((i>0)&&(i<25))
                    if((u>=0)&&(u<200))
                        if((v>=0)&&(v<200)) {
                            Vec3b pixsrc=imgsrc.at<Vec3b>(row,col); // image pixel
                            Vec4f texsrc=tiles[i-1].at<Vec4f>(u,v);
                            texsrc[0]+=(float)pixsrc[0];
                            texsrc[1]+=(float)pixsrc[1];
                            texsrc[2]+=(float)pixsrc[2];
                            texsrc[3]+=(float)1.0;
                            tiles[i-1].at<Vec4f>(u,v)=texsrc;
                            //tiles[i-1].at<Vec3b>(u,v)=imgsrc.at<Vec3b>(row,col);
                        }
            }
        cumul=cumul+texture;
        count+=1;
        for(int row=0; row<1200; row++)
            for(int col=0; col<800; col++) {
                Vec4f texsrc=texture.at<Vec4f>(row,col);
                texsrc[0]=float(texsrc[0])/float(texsrc[3]);
                texsrc[1]=float(texsrc[1])/float(texsrc[3]);
                texsrc[2]=float(texsrc[2])/float(texsrc[3]);
                texsrc[3]=(float)(texsrc[3] ? 255.0:0.0);
                texture.at<Vec4f>(row,col)=texsrc;
            }
        imwrite(teximpath.c_str(),texture);

        cout << endl;
    }

    for(int row=0; row<1200; row++)
        for(int col=0; col<800; col++) {
            Vec4f texsrc=cumul.at<Vec4f>(row,col);
            texsrc[0]=float(texsrc[0])/float(texsrc[3]);
            texsrc[1]=float(texsrc[1])/float(texsrc[3]);
            texsrc[2]=float(texsrc[2])/float(texsrc[3]);
            //texsrc[3]=(float)255.0;
            texsrc[3]=(float)(texsrc[3] ? 255.0:0.0);
            cumul.at<Vec4f>(row,col)=texsrc;
        }
    imwrite("cumul.png",cumul);

    return 0;
}
