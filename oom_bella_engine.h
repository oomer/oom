#pragma once

// Bella SDK includes - external libraries for 3D rendering
#include "../bella_engine_sdk/src/bella_sdk/bella_engine.h" // For creating and manipulating 3D scenes in Bella

// Define the oom namespace
namespace oom {
    namespace bella {
        // Function declaration

        /*
        * Generic MyEngineObserver Class override
        * This class receives callbacks from the Bella rendering engine to track rendering progress.
        * It implements the EngineObserver interface and provides methods to:
        * - Handle render start/stop events
        * - Track rendering progress
        * - Handle error conditions
        * - Store and retrieve the current progress state
        */
        struct MyEngineObserver : public dl::bella_sdk::EngineObserver
        {
        public:
            // Called when a rendering pass starts
            void onStarted(dl::String pass) override
            {
                dl::logInfo("Started pass %s", pass.buf());
            }

            // Called to update the current status of rendering
            void onStatus(dl::String pass, dl::String status) override
            {
                dl::logInfo("%s [%s]", status.buf(), pass.buf());
            }

            // Called to update rendering progress (percentage, time remaining, etc)
            void onProgress(dl::String pass, dl::bella_sdk::Progress progress) override
            {
                dl::logInfo("%s [%s]", progress.toString().buf(), pass.buf());
            }

            void onImage(dl::String pass, dl::bella_sdk::Image image) override
            {
                dl::logInfo("We got an image %d x %d.", (int)image.width(), (int)image.height());
            }  

            // Called when an error occurs during rendering
            void onError(dl::String pass, dl::String msg) override
            {
                dl::logError("%s [%s]", msg.buf(), pass.buf());
            }

            // Called when a rendering pass completes
            void onStopped(dl::String pass) override
            {
                dl::logInfo("Stopped %s", pass.buf());
            }

            // Returns the current progress as a string
            std::string getProgress() const {
                std::string* currentProgress = progressPtr.load();
                if (currentProgress) {
                    return *currentProgress;
                } else {
                    return "";
                }
            }

            // Cleanup resources in destructor
            ~MyEngineObserver() {
                setString(nullptr);
            }
        private:
            // Thread-safe pointer to current progress string
            std::atomic<std::string*> progressPtr{nullptr};

            // Helper function to safely update the progress string
            void setString(std::string* newStatus) {
                std::string* oldStatus = progressPtr.exchange(newStatus);
                delete oldStatus;  // Clean up old string if it exists
            }
        };

    }
}
