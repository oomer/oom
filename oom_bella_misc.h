#pragma once

extern int s_oomBellaLogContext; //  Declare the s_dlLogContext variable

namespace oom {
    namespace bella {

        // Define color codes
        #define OOMRESET   "\033[0m"
        #define OOMRED     "\033[31m"
        #define OOMGREEN   "\033[32m"
        #define OOMYELLOW  "\033[33m"
        #define OOMBLUE    "\033[34m"

        // In your log function:
        static void log(void* /*ctx*/, dl::LogType type, const char* msg)
        {
            switch (type)
            {
            case dl::LogType_Info:
                DL_PRINT(OOMGREEN "[INFO] %s" OOMRESET "\n", msg);
                break;
            case dl::LogType_Warning:
                if (strcmp(msg, "Redefining arg with long form: 'input'") == 0) {
                    break;
                } else if (strcmp(msg, "Core node implementations are not linked.") == 0) {
                    break;
                } else if (strcmp(msg, "Redefining arg with short form: 'o'") == 0) {
                    break;
                }


                DL_PRINT(OOMYELLOW "[WARN] %s" OOMRESET "\n", msg);
                break;
            case dl::LogType_Error:
                DL_PRINT(OOMRED "[ERROR] %s" OOMRESET "\n", msg);
                break;
            case dl::LogType_Custom:
                DL_PRINT(OOMBLUE "%s\n" OOMRESET, msg);
                break;
            }
        }


        void zoomToScene(dl::bella_sdk::Engine engine, bool voxel = false) {
            dl::Vec3f min, max;
            engine.start();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (voxel) {
                engine.instancesBoundingBoxes( min, max, {} ); // Get the bounding boxes of all instances in the scenea
            } else {
                dl::bella_sdk::Scene belScene = engine.scene();
                dl::ds::UInt64Vector vHashes;
                dl::StringVector useTypes = dl::StringVector(); 
                useTypes.push_back("mesh");
                useTypes.push_back("xform");
                useTypes.push_back("instancer");
                useTypes.push_back("box");
                dl::bella_sdk::PathVector paths = belScene.world().paths(useTypes);
                for (const auto& eachPath : paths) {
                    std::cout << eachPath.path().buf() << std::endl;
                    std::cout << eachPath.hash() << "\n"<< std::endl;
                    vHashes.push_back(eachPath.path().hash());
                }
                engine.instancesBoundingBoxes( min, max, vHashes ); // Get the bounding boxes of all instances in the scenea
            }
            std::cout << "min: " << min << std::endl;
            std::cout << "max: " << max << std::endl;
            dl::Vec3f centerf = ( min + max ) * 0.5f;
            float radius = norm(max - min ) * 0.5f;
            dl::Vec3d center = dl::Vec3d::make( centerf.x, centerf.y, centerf.z );
            auto cameraPath = engine.scene().cameraPath();
            dl::bella_sdk::zoomExtents( cameraPath, center, radius );
            engine.stop();
        }

        void setOutputImagePath(dl::bella_sdk::Engine engine ) {
            auto belScene = engine.scene();
            auto belOutputImagePath = belScene.createNode("outputImagePath","oomOutputImagePath","oomOutputImagePath");
            belOutputImagePath["ext"] = ".png";
            belOutputImagePath["dir"] = ".";
            belScene.beautyPass()["overridePath"] = belOutputImagePath;
        }
    }
}