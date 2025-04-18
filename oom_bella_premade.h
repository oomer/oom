#pragma once

#include <tuple>

// Bella SDK includes - external libraries for 3D rendering
#include "../bella_engine_sdk/src/bella_sdk/bella_engine.h" // For creating and manipulating 3D scenes in Bella
#include "../oom/oom_bella_long.h" // This file is very large
#include "../oom/oom_misc.h" // This file is very large

// Define the oom namespace
namespace oom {
    namespace bella {
        // Function declaration
        dl::bella_sdk::Node defaultScene2025(dl::bella_sdk::Scene& belScene);
        //dl::bella_sdk::Node defaultSceneVoxel(dl::bella_sdk::Scene& belScene);
        std::tuple< dl::bella_sdk::Node, 
                    dl::bella_sdk::Node, 
                    dl::bella_sdk::Node, 
                    dl::bella_sdk::Node, 
                    dl::bella_sdk::Node> defaultSceneVoxel(dl::bella_sdk::Scene& belScene);

        // @param belScene - the scene to create the essentials in
        // @return - the world node
        dl::bella_sdk::Node defaultScene2025(dl::bella_sdk::Scene& belScene) {
            // Create the basic scene elements in Bella
            // Each line creates a different type of node in the scene auto belBeautyPass     = belScene.createNode("beautyPass","oomerBeautyPass","oomerBeautyPass");

            oom::misc::saveHDRI();

            auto belWorld = belScene.world();       // Get scene world root
            {
                dl::bella_sdk::Scene::EventScope es(belScene);
                // Configure camera
                auto belCamForm         = belScene.createNode("xform","oomCameraXform","oomCameraXform");
                auto belCam             = belScene.createNode("camera","oomCamera","oomCamera");
                auto belSensor          = belScene.createNode("sensor","oomSensor","oomSensor");
                auto belLens            = belScene.createNode("thinLens","oomThinLens","oomThinLens");
                belCam["resolution"]    = dl::Vec2 {800, 800};  // Set resolution to 1080p
                belCam["lens"]          = belLens;               // Connect camera to lens
                belCam["sensor"]        = belSensor;             // Connect camera to sensor
                belCamForm.parentTo(belWorld);                  // Parent camera transform to world
                belCam.parentTo(belCamForm);                   // Parent camera to camera transform
                // Position the camera with a transformation matrix
                belCamForm["steps"][0]["xform"] = dl::Mat4 {0.525768608156, -0.850627633385, 0, 0, -0.234464751651, -0.144921468924, -0.961261695938, 0, 0.817675761479, 0.505401223947, -0.275637355817, 0, -88.12259018466, -54.468125200218, 50.706001690932, 1};

                // Configure environment (image-based lighting)
                auto belImageDome               = belScene.createNode("imageDome","oomImageDome","oomImageDome");
                auto belColorDome       = belScene.createNode("colorDome","oomColorDome","oomColorDome");
                belImageDome["ext"]             = ".jpg";
                belImageDome["dir"]             = "./res";
                belImageDome["multiplier"]      = 6.0f;
                belImageDome["file"]            = "DayEnvironmentHDRI019_1K-TONEMAPPED";
                belImageDome["overrides"]["background"]     = belColorDome;
                belColorDome["zenith"]          = dl::Rgba{1.0f, 1.0f, 1.0f, 1.0f};
                belColorDome["horizon"]         = dl::Rgba{.85f, 0.76f, 0.294f, 1.0f};
                belColorDome["altitude"]        = 14.0f;

                // Configure a metallic ground plane
                auto belGroundPlane     = belScene.createNode("groundPlane","oomGroundPlane","oomGroundPlane");
                auto belGroundMat       = belScene.createNode("quickMaterial","oomGroundMat","oomGroundMat");
                belGroundPlane["material"]      = belGroundMat;
                belGroundMat["type"] = "metal";
                belGroundMat["roughness"] = 22.0f;
                belGroundMat["color"] = dl::Rgba{0.138431623578, 0.5, 0.3, 1.0};

                auto belOutputImagePath = belScene.createNode("outputImagePath","oomOutputImagePath","oomOutputImagePath");
                belOutputImagePath["ext"] = ".png";
                belOutputImagePath["dir"] = ".";

                // Set up scene settings
                auto belSettings        = belScene.settings(); // Get scene settings
                auto belBeautyPass      = belScene.createNode("beautyPass","oomBeautyPass","oomBeautyPass");
                belBeautyPass["overridePath"] = belOutputImagePath;
                belSettings["beautyPass"]  = belBeautyPass;
                belSettings["camera"]      = belCam;
                belSettings["environment"] = belImageDome;
                belSettings["iprScale"]    = 100.0f;
                belSettings["threads"]     = dl::bella_sdk::Input(0);  // Auto-detect thread count
                belSettings["groundPlane"] = belGroundPlane;
                belSettings["iprNavigation"] = "maya";  // Use Maya-like navigation in viewer
            }
            return belWorld;
        }

        // @param belScene - the scene to create the essentials in
        // @return - a tuple of the world node, the voxel node, the liquid voxel node, and the mesh voxel node
        // The mesh voxel node ia actually a hiearchy of a mesh node and a smaller procedural box inside it
        std::tuple< dl::bella_sdk::Node, 
                    dl::bella_sdk::Node, 
                    dl::bella_sdk::Node, 
                    dl::bella_sdk::Node,
                    dl::bella_sdk::Node> defaultSceneVoxel(dl::bella_sdk::Scene& belScene) {
            // Create basic scene elements in Bella for voxel rendering
            auto belWorld     = belScene.world();       // Get scene world root
            auto belVoxel     = belScene.createNode("box","oomVoxel","oomVoxel");
            auto belEmitterBlockXform     = belScene.createNode("xform","oomEmitterBlockXform","oomEmitterBlockXform");
            auto belLiqVoxel  = belScene.createNode("box","oomLiqVoxel","oomLiqVoxel");
            auto belMeshVoxel = addMeshCube(belScene, "oomMeshVoxel");
            oom::misc::saveHDRI();
            
            {
                dl::bella_sdk::Scene::EventScope es(belScene);

                // Configure camera
                auto belCamForm         = belScene.createNode("xform","oomCameraXform","oomCameraXform");
                auto belCam             = belScene.createNode("camera","oomCamera","oomCamera");
                auto belSensor          = belScene.createNode("sensor","oomSensor","oomSensor");
                auto belLens            = belScene.createNode("thinLens","oomThinLens","oomThinLens");
                belCam["resolution"]    = dl::Vec2 {800, 800};  // Set resolution to 1080p
                belCam["lens"]          = belLens;               // Connect camera to lens
                belCam["sensor"]        = belSensor;             // Connect camera to sensor
                belCamForm.parentTo(belWorld);                  // Parent camera transform to world
                belCam.parentTo(belCamForm);                   // Parent camera to camera transform

                // Position the camera with a transformation matrix
                belCamForm["steps"][0]["xform"] = dl::Mat4 {0.525768608156, -0.850627633385, 0, 0, -0.234464751651, -0.144921468924, -0.961261695938, 0, 0.817675761479, 0.505401223947, -0.275637355817, 0, -88.12259018466, -54.468125200218, 50.706001690932, 1};

                // Configure environment (image-based lighting)
                auto belImageDome       = belScene.createNode("imageDome","oomImageDome","oomImageDome");
                auto belColorDome       = belScene.createNode("colorDome","oomColorDome","oomColorDome");
                belImageDome["ext"]         = ".jpg";
                belImageDome["dir"]         = "./res";
                belImageDome["multiplier"]  = 6.0f;
                belImageDome["file"]        = "DayEnvironmentHDRI019_1K-TONEMAPPED";
                belImageDome["overrides"]["background"] = belColorDome;
                belColorDome["zenith"]      = dl::Rgba{1.0f, 1.0f, 1.0f, 1.0f};
                belColorDome["horizon"]     = dl::Rgba{.85f, 0.76f, 0.294f, 1.0f};
                belColorDome["altitude"]    = 14.0f;

                // Configure ground plane
                auto belGroundPlane     = belScene.createNode("groundPlane","oomGroundPlane","oomGroundPlane");
                auto belGroundMat       = belScene.createNode("quickMaterial","oomGroundMat","oomGroundMat");
                belGroundPlane["material"]  = belGroundMat;
                belGroundMat["type"] = "metal";
                belGroundMat["roughness"] = 22.0f;
                belGroundMat["color"] = dl::Rgba{0.138431623578, 0.5, 0.3, 1.0};

                auto belOutputImagePath = belScene.createNode("outputImagePath","oomOutputImagePath","oomOutputImagePath");
                belOutputImagePath["ext"] = ".png";
                belOutputImagePath["dir"] = ".";

                // Set up scene settings
                auto belSettings            = belScene.settings(); // Get scene settings
                auto belBeautyPass          = belScene.createNode("beautyPass","oomBeautyPass","oomBeautyPass");
                belBeautyPass["overridePath"] = belOutputImagePath;
                belSettings["beautyPass"]   = belBeautyPass;
                belSettings["camera"]       = belCam;
                belSettings["environment"]  = belImageDome;
                belSettings["iprScale"]     = 100.0f;
                belSettings["threads"]      = dl::bella_sdk::Input(0);  // Auto-detect thread count
                belSettings["groundPlane"]  = belGroundPlane;
                belSettings["iprNavigation"]= "maya";  // Use Maya-like navigation in viewer

                // Create voxel nodes
                //auto belVoxelForm      = belScene.createNode("xform","oomVoxelXform","oomVoxelXform");
                //auto belLiqVoxelForm   = belScene.createNode("xform","oomLiqVoxelXform","oomLiqVoxelXform");
                auto belEmitterBlockMat   = belScene.createNode("orenNayar","oomEmitterBlockMat","oomEmitterBlockMat");
                auto belBevel = belScene.createNode("bevel", "oomBevel", "oomBevel");
                belBevel["radius"] = 90.0f;
                belBevel["samples"] =dl::UInt(6); 
                belVoxel["radius"]  = 0.33f;
                belVoxel["sizeX"]   = 0.99f;
                belVoxel["sizeY"]   = 0.99f;
                belVoxel["sizeZ"]   = 0.99f;

                belVoxel.parentTo(belEmitterBlockXform);
                belEmitterBlockXform["steps"][0]["xform"] = dl::Mat4 {0.999,0,0,0,0,0.999,0,0,0,0,0.999,0,0,0,0,1};
                belEmitterBlockMat["reflectance"] = dl::Rgba{0.0, 0.0, 0.0, 1.0};
                belEmitterBlockXform["material"] = belEmitterBlockMat;


                // Less gap to make liquid look better, allows more light to pass through
                belLiqVoxel["sizeX"]    = 0.99945f;
                belLiqVoxel["sizeY"]    = 0.99945f;
                belLiqVoxel["sizeZ"]    = 0.99945f;
                //belVoxel.parentTo(belVoxelForm);
                //belVoxelForm["steps"][0]["xform"] = dl::Mat4 {0.999,0,0,0,0,0.999,0,0,0,0,0.999,0,0,0,0,1};
                //belVoxelMat["reflectance"] = dl::Rgba{0.0, 0.0, 0.0, 1.0};
                //belVoxelForm["material"] = belVoxelMat;
            }
            return std::make_tuple(belWorld, belMeshVoxel, belLiqVoxel, belVoxel, belEmitterBlockXform);
        }
    }
}
