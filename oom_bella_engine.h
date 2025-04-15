#pragma once

// Bella SDK includes - external libraries for 3D rendering
#include "../bella_engine_sdk/src/bella_sdk/bella_engine.h" // For creating and manipulating 3D scenes in Bella

// Define the oom namespace
namespace oom {
    namespace bella {
        // Function declaration
        dl::bella_sdk::Node defaultScene2025(dl::bella_sdk::Scene& belScene);

        // @param belScene - the scene to create the essentials in
        // @return - the world node
        dl::bella_sdk::Node defaultScene2025(dl::bella_sdk::Scene& belScene) {
            // Create the basic scene elements in Bella
            // Each line creates a different type of node in the scene auto belBeautyPass     = belScene.createNode("beautyPass","oomerBeautyPass","oomerBeautyPass");
            auto belWorld = belScene.world();       // Get scene world root
            {
                dl::bella_sdk::Scene::EventScope es(belScene);

                auto belCamForm    = belScene.createNode("xform","oomerCameraXform","oomerCameraXform");
                auto belCam        = belScene.createNode("camera","oomerCamera","oomerCamera");
                auto belSensor         = belScene.createNode("sensor","oomerSensor","oomerSensor");
                auto belLens           = belScene.createNode("thinLens","oomerThinLens","oomerThinLens");
                auto belImageDome      = belScene.createNode("imageDome","oomerImageDome","oomerImageDome");
                auto belGroundPlane    = belScene.createNode("groundPlane","oomerGroundPlane","oomerGroundPlane");

                auto belBeautyPass     = belScene.createNode("beautyPass","oomerBeautyPass","oomerBeautyPass");
                auto belGroundMat      = belScene.createNode("quickMaterial","oomerGroundMat","oomerGroundMat");
                auto belSun            = belScene.createNode("sun","oomerSun","oomerSun");
                auto belColorDome   = belScene.createNode("colorDome","oomerColorDome","oomerColorDome");
                auto belSettings = belScene.settings(); // Get scene settings
                // Configure camera
                belCam["resolution"]    = dl::Vec2 {1920, 1080};  // Set resolution to 1080p
                belCam["lens"]          = belLens;               // Connect camera to lens
                belCam["sensor"]        = belSensor;             // Connect camera to sensor
                belCamForm.parentTo(belWorld);                  // Parent camera transform to world
                belCam.parentTo(belCamForm);                   // Parent camera to camera transform

                // Position the camera with a transformation matrix
                belCamForm["steps"][0]["xform"] = dl::Mat4 {0.525768608156, -0.850627633385, 0, 0, -0.234464751651, -0.144921468924, -0.961261695938, 0, 0.817675761479, 0.505401223947, -0.275637355817, 0, -88.12259018466, -54.468125200218, 50.706001690932, 1};

                // Configure environment (image-based lighting)
                belImageDome["ext"]            = ".jpg";
                belImageDome["dir"]            = "./res";
                belImageDome["multiplier"]     = 6.0f;
                belImageDome["file"]           = "DayEnvironmentHDRI019_1K-TONEMAPPED";
                belImageDome["overrides"]["background"]     = belColorDome;
                belColorDome["zenith"] = dl::Rgba{1.0f, 1.0f, 1.0f, 1.0f};
                belColorDome["horizon"] = dl::Rgba{.85f, 0.76f, 0.294f, 1.0f};
                belColorDome["altitude"] = 14.0f;
                // Configure ground plane
                //belGroundPlane["elevation"]    = -.5f;
                belGroundPlane["material"]     = belGroundMat;

                /* Commented out: Sun configuration
                belSun["size"]    = 20.0f;
                belSun["month"]    = "july";
                belSun["rotation"]    = 50.0f;*/

                // Configure materials
                belGroundMat["type"] = "metal";
                belGroundMat["roughness"] = 22.0f;
                belGroundMat["color"] = dl::Rgba{0.138431623578, 0.5, 0.3, 1.0};

                // Set up scene settings
                belSettings["beautyPass"]  = belBeautyPass;
                belSettings["camera"]      = belCam;
                belSettings["environment"] = belColorDome;
                belSettings["iprScale"]    = 100.0f;
                belSettings["threads"]     = dl::bella_sdk::Input(0);  // Auto-detect thread count
                belSettings["groundPlane"] = belGroundPlane;
                belSettings["iprNavigation"] = "maya";  // Use Maya-like navigation in viewer
                //settings["sun"] = sun;

                auto belVoxel          = belScene.createNode("box","oomerVoxel","oomerVoxel");
                auto belLiqVoxel       = belScene.createNode("box","oomerLiqVoxel","oomerLiqVoxel");
                auto belVoxelForm      = belScene.createNode("xform","oomerVoxelXform","oomerVoxelXform");
                auto belLiqVoxelForm   = belScene.createNode("xform","oomerLiqVoxelXform","oomerLiqVoxelXform");
                auto belVoxelMat       = belScene.createNode("orenNayar","oomerVoxelMat","oomerVoxelMat");
                auto belMeshVoxel   = belScene.createNode("mesh", "oomerMeshVoxel");
                auto belBevel = belScene.createNode("bevel", "oomerBevel");
                belBevel["radius"] = 90.0f;
                belBevel["samples"] =dl::UInt(6); 

                //#include "resources/smoothcube.h"
                addMeshCube(belMeshVoxel);
               // Configure voxel box dimensions
                belVoxel["radius"]  = 0.33f;
                belVoxel["sizeX"]   = 0.99f;
                belVoxel["sizeY"]   = 0.99f;
                belVoxel["sizeZ"]   = 0.99f;

                // Less gap to make liquid look better, allows more light to pass through
                belLiqVoxel["sizeX"]    = 0.99945f;
                belLiqVoxel["sizeY"]    = 0.99945f;
                belLiqVoxel["sizeZ"]    = 0.99945f;

                belVoxel.parentTo(belVoxelForm);
                belVoxelForm["steps"][0]["xform"] = dl::Mat4 {0.999,0,0,0,0,0.999,0,0,0,0,0.999,0,0,0,0,1};
                belVoxelMat["reflectance"] = dl::Rgba{0.0, 0.0, 0.0, 1.0};
                belVoxelForm["material"] = belVoxelMat;
            }
            return belWorld;
        }
    }
}
