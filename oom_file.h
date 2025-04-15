#pragma once

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>
#include <filesystem>

#include <string>
#include <atomic>
#include <mutex> // Add this line for std::mutex and std::lock_guard
#include <map> // Add this line for std::map

#include "../efsw/src/efsw/FileSystem.hpp" // For file watching
#include "../efsw/src/efsw/System.hpp" // For file watching
#include "../efsw/include/efsw/efsw.hpp" // For file watching


//==============================================================================
// OOM NAMESPACE - Contains core file watching functionality
//==============================================================================
namespace oom {
    namespace file {
        //--------------------------------------------------------------------------
        // RenderQueue Class
        //--------------------------------------------------------------------------
        /**
        * A thread-safe FIFO queue that tracks render files
        * 
        * This class implements a specialized queue that maintains both:
        * 1. FIFO (First-In-First-Out) ordering via a vector 
        * 2. Fast lookup capability via a map
        * 
        * It's designed to be safely used across multiple threads
        */
        class Queue {
        public:
            // Default constructor - Creates an empty queue
            Queue() = default;
        
            // Move constructor - Transfers ownership of resources from another queue
            Queue(Queue&& other) noexcept {
                // Lock the other queue to prevent concurrent modification during the move
                std::lock_guard<std::mutex> lock(other.mutex);
                
                // Transfer ownership of the data structures using std::move
                pathVector = std::move(other.pathVector);
                pathMap = std::move(other.pathMap);
            }
        
            // Move assignment operator - Similar to move constructor but for assignment operations
            Queue& operator=(Queue&& other) noexcept {
                // Avoid self-assignment (moving to itself)
                if (this != &other) {
                    // Lock both queues to prevent concurrent modification
                    std::lock_guard<std::mutex> lock1(mutex);
                    std::lock_guard<std::mutex> lock2(other.mutex);
                    
                    // Transfer ownership of the data structures
                    pathVector = std::move(other.pathVector);
                    pathMap = std::move(other.pathMap);
                }
                return *this;
            }
        
            // Delete copy operations since mutexes can't be copied
            Queue(const Queue&) = delete;
            Queue& operator=(const Queue&) = delete;
        
            // Add a file to the queue if it's not already there
            bool push(const std::filesystem::path& path) {
                // Lock the queue during modification to ensure thread safety
                std::lock_guard<std::mutex> lock(mutex);
                
                // Check if the path already exists in our map (fast lookup)
                if (pathMap.find(path) == pathMap.end()) {
                    // If not found, add to the vector (maintains order)
                    pathVector.push_back(path);
                    // Mark as present in the map for fast future lookups
                    pathMap[path] = true;
                    return true;
                }
                return false;
            }
        
            // Get the next file to render (FIFO order) and remove it from the queue
            bool pop(std::filesystem::path& outPath) {
                // Lock the queue during access to ensure thread safety
                std::lock_guard<std::mutex> lock(mutex);
                
                if (!pathVector.empty()) {
                    // Get the first item (oldest) from the vector
                    outPath = pathVector.front();
                    // Remove it from the vector
                    pathVector.erase(pathVector.begin());
                    // Remove it from the map as well
                    pathMap.erase(outPath);
                    return true;
                }
                return false;
            }
        
            // Remove a specific file by name
            bool remove(const std::filesystem::path& path) {
                // Lock the queue during modification to ensure thread safety
                std::lock_guard<std::mutex> lock(mutex);
                
                if (pathMap.find(path) != pathMap.end()) {
                    // Remove from vector using erase-remove idiom
                    pathVector.erase(
                        std::remove(pathVector.begin(), pathVector.end(), path),
                        pathVector.end()
                    );
                    // Remove from map
                    pathMap.erase(path);
                    return true;
                }
                return false;
            }
        
            // Check if a file exists in the queue
            bool contains(const std::filesystem::path& path) const {
                // Lock the queue during access to ensure thread safety
                std::lock_guard<std::mutex> lock(mutex);
                // Use the map for fast lookup
                return pathMap.find(path) != pathMap.end();
            }
        
            // Get the number of files in the queue
            size_t size() const {
                // Lock the queue during access to ensure thread safety
                std::lock_guard<std::mutex> lock(mutex);
                return pathVector.size();
            }
        
            // Check if the queue is empty
            bool empty() const {
                // Lock the queue during access to ensure thread safety
                std::lock_guard<std::mutex> lock(mutex);
                return pathVector.empty();
            }
        
            // Clear all files from the queue
            void clear() {
                // Lock the queue during modification to ensure thread safety
                std::lock_guard<std::mutex> lock(mutex);
                // Clear both data structures
                pathVector.clear();
                pathMap.clear();
            }
        
        private:
            std::vector<std::filesystem::path> pathVector;  // Stores paths in order (FIFO queue)
            std::map<std::filesystem::path, bool> pathMap;  // Maps paths to bool for fast existence checks
            mutable std::mutex mutex;            // Makes operations thread-safe; mutable allows locking in const methods
        };

        // Forward declaration needed for UpdateListener
        class Watcher;

        //--------------------------------------------------------------------------
        // UpdateListener Class
        //--------------------------------------------------------------------------
        /**
        * Processes file system events from efsw and filters them
        * 
        * This class receives notifications from the file system watcher (efsw)
        * when files are created, modified or deleted, and then filters these
        * events based on file extensions and directories.
        */
        class UpdateListener : public efsw::FileWatchListener {
        public:
            // Constructor takes a pointer to its parent Watcher
            UpdateListener(Watcher* parent);
            
            // Signals the listener to stop processing events
            void stop();
            
            // Converts efsw action codes to human-readable strings
            std::string getActionName(efsw::Action action);
            
            // Check if a file extension is in our watch list
            bool isWatchedExtension(const std::string& extension);
            
            // Check if a directory should be ignored
            bool isIgnoredDirectory(const std::string& directory);
            
            // Main callback that efsw calls when file system events occur
            void handleFileAction(efsw::WatchID watchid, 
                                const std::string& dir,
                                const std::string& filename, 
                                efsw::Action action,
                                std::string oldFilename = "") override;
        private:
            std::atomic<bool> should_stop_{false};    // Flag to signal stopping
            Watcher* parentWatcher;               // Reference to parent Watcher
        };

        //--------------------------------------------------------------------------
        // Watcher Class
        //--------------------------------------------------------------------------
        /**
        * Manages file watching and file queues
        * 
        * This class coordinates the file system watching functionality
        * and maintains queues of files to be rendered or deleted.
        */
        class Watcher {
        public:
            // Constructor with configurable extensions and ignored directories
            Watcher(const std::vector<std::string>& extensions = {".bsz", ".zip"},
                        const std::vector<std::string>& ignoreDirs = {"download"});
            
            // Destructor to clean up resources
            ~Watcher();

            // Prevent copying or moving to simplify handling
            Watcher(const Watcher&) = delete;
            Watcher& operator=(const Watcher&) = delete;
            Watcher(Watcher&&) = delete;
            Watcher& operator=(Watcher&&) = delete;
            
            // Start watching a directory
            bool startWatching(const std::string& directory);
            
            // Stop watching
            void stopWatching();
            
            // Get the next file to render
            bool getNextFileToRender(std::filesystem::path& outPath);
            
            // Get the next file to delete
            bool getNextFileToDelete(std::filesystem::path& outPath);
            
            // Check if there are files to render
            bool hasFilesToRender() const;
            
            // Check if there are files to delete
            bool hasFilesToDelete() const;
            
            // Add or remove extensions dynamically
            void addExtension(const std::string& extension);
            void removeExtension(const std::string& extension);
            
            // Add or remove ignored directories
            void addIgnoreDirectory(const std::string& directory);
            void removeIgnoreDirectory(const std::string& directory);
            
            // Get extensions and ignored directories (for UpdateListener)
            const std::vector<std::string>& getWatchExtensions() const { return watchExtensions; }
            const std::vector<std::string>& getIgnoreDirectories() const { return ignoreDirectories; }
            
            // Access to queues (for UpdateListener)
            void addToRenderQueue(const std::filesystem::path& path);
            void addToDeleteQueue(const std::filesystem::path& path);
            
        private:
            // File extensions to monitor (without leading dot)
            std::vector<std::string> watchExtensions;
            
            // Directory names to ignore
            std::vector<std::string> ignoreDirectories;
            
            // Queues for the different file actions
            Queue incomingRenderQueue;  
            Queue incomingDeleteQueue;
            
            // Mutexes for thread safety
            mutable std::mutex renderQueueMutex;
            mutable std::mutex deleteQueueMutex;
            mutable std::mutex extensionsMutex;
            mutable std::mutex directoriesMutex;
            
            // File watcher components
            std::unique_ptr<efsw::FileWatcher> fileWatcher;
            std::unique_ptr<UpdateListener> updateListener;
            
            // Watch state
            bool isWatching;
            std::atomic<bool> stopRequested;
            std::thread watcherThread;
            
            // Path being watched
            std::string watchPath;
            
            // Internal function to run the watcher in a separate thread
            void watcherThreadFunc();
        };

        //--------------------------------------------------------------------------
        // Global variable within oom namespace
        //--------------------------------------------------------------------------
        std::atomic<bool> active_render(false);

        //==========================================================================
        // IMPLEMENTATION SECTION - Method implementations for the above classes
        //==========================================================================

        //--------------------------------------------------------------------------
        // UpdateListener implementation
        //--------------------------------------------------------------------------
        
        UpdateListener::UpdateListener(Watcher* parent) 
            : should_stop_(false), parentWatcher(parent) {}

        void UpdateListener::stop() {
            should_stop_ = true;
        }

        std::string UpdateListener::getActionName(efsw::Action action) {
            switch (action) {
                case efsw::Actions::Add:
                    return "Add";
                case efsw::Actions::Modified:
                    return "Modified";
                case efsw::Actions::Delete:
                    return "Delete";
                case efsw::Actions::Moved:
                    return "Moved";
                default:
                    return "Bad Action";
            }
        }

        bool UpdateListener::isWatchedExtension(const std::string& extension) {
            // Get the extensions list from parent
            const auto& extensions = parentWatcher->getWatchExtensions();
            
            // Check if the extension is in our watch list
            return std::find(extensions.begin(), extensions.end(), extension) != extensions.end();
        }

        bool UpdateListener::isIgnoredDirectory(const std::string& directory) {
            // Get the ignored directories list from parent
            const auto& ignoreDirs = parentWatcher->getIgnoreDirectories();
            
            // Check if the directory is in our ignore list
            return std::find(ignoreDirs.begin(), ignoreDirs.end(), directory) != ignoreDirs.end();
        }

        void UpdateListener::handleFileAction(efsw::WatchID watchid, 
                                            const std::string& dir,
                                            const std::string& filename, 
                                            efsw::Action action,
                                            std::string oldFilename) {
            if (should_stop_) return;  // Early exit if we're stopping
            
            std::string actionName = getActionName(action);
            std::filesystem::path filePath = dir + filename;
            std::filesystem::path parentPath = dir;
            
            // Get the filename extension without the dot
            std::string extension = filePath.extension().string();
            if (!extension.empty() && extension[0] == '.') {
                extension = extension.substr(1);
            }
            
            // Get the parent directory name
            std::string parentDir = parentPath.filename().string();
            
            // Check if we should ignore this directory
            if (isIgnoredDirectory(parentDir)) {
                return;
            }
            
            // Process based on the action type
            if (actionName == "Delete") {
                if (isWatchedExtension(extension)) {
                    parentWatcher->addToDeleteQueue(filePath);
                    std::cout << "\n==" << "STOP RENDER: " << filePath.string() << "\n==" << std::endl;
                }
            } else if (actionName == "Add" || actionName == "Modified") {
                if (should_stop_) return;  // Check again before starting render
                
                if (isWatchedExtension(extension)) {
                    parentWatcher->addToRenderQueue(filePath);
                    std::cout << "\n==" << "RENDER QUEUED: " << filePath.string() << "\n==" << std::endl;
                }
            }
        }

        //--------------------------------------------------------------------------
        // Watcher implementation
        //--------------------------------------------------------------------------
        
        Watcher::Watcher(const std::vector<std::string>& extensions,
                            const std::vector<std::string>& ignoreDirs)
            : isWatching(false), stopRequested(false) {
            // Process extensions to ensure they don't have leading dots
            for (const auto& ext : extensions) {
                std::string processedExt = ext;
                if (!ext.empty() && ext[0] == '.') {
                    processedExt = ext.substr(1);
                }
                watchExtensions.push_back(processedExt);
            }
            
            // Store ignored directories
            ignoreDirectories = ignoreDirs;
        }

        Watcher::~Watcher() {
            stopWatching();
        }

        bool Watcher::startWatching(const std::string& directory) {
            if (isWatching) {
                stopWatching();
            }
            
            watchPath = directory;
            stopRequested = false;
            
            // Start the watcher thread
            watcherThread = std::thread(&Watcher::watcherThreadFunc, this);
            isWatching = true;
            
            return true;
        }

        void Watcher::stopWatching() {
            if (!isWatching) return;
            
            stopRequested = true;
            
            if (updateListener) {
                updateListener->stop();
            }
            
            if (watcherThread.joinable()) {
                watcherThread.join();
            }
            
            isWatching = false;
        }

        bool Watcher::getNextFileToRender(std::filesystem::path& outPath) {
            std::lock_guard<std::mutex> lock(renderQueueMutex);
            return incomingRenderQueue.pop(outPath);
        }

        bool Watcher::getNextFileToDelete(std::filesystem::path& outPath) {
            std::lock_guard<std::mutex> lock(deleteQueueMutex);
            return incomingDeleteQueue.pop(outPath);
        }

        bool Watcher::hasFilesToRender() const {
            std::lock_guard<std::mutex> lock(renderQueueMutex);
            return !incomingRenderQueue.empty();
        }

        bool Watcher::hasFilesToDelete() const {
            std::lock_guard<std::mutex> lock(deleteQueueMutex);
            return !incomingDeleteQueue.empty();
        }

        void Watcher::addExtension(const std::string& extension) {
            std::lock_guard<std::mutex> lock(extensionsMutex);
            std::string processedExt = extension;
            if (!extension.empty() && extension[0] == '.') {
                processedExt = extension.substr(1);
            }
            
            // Check if it already exists
            if (std::find(watchExtensions.begin(), watchExtensions.end(), processedExt) == watchExtensions.end()) {
                watchExtensions.push_back(processedExt);
            }
        }

        void Watcher::removeExtension(const std::string& extension) {
            std::lock_guard<std::mutex> lock(extensionsMutex);
            std::string processedExt = extension;
            if (!extension.empty() && extension[0] == '.') {
                processedExt = extension.substr(1);
            }
            
            auto it = std::find(watchExtensions.begin(), watchExtensions.end(), processedExt);
            if (it != watchExtensions.end()) {
                watchExtensions.erase(it);
            }
        }

        void Watcher::addIgnoreDirectory(const std::string& directory) {
            std::lock_guard<std::mutex> lock(directoriesMutex);
            
            // Check if it already exists
            if (std::find(ignoreDirectories.begin(), ignoreDirectories.end(), directory) == ignoreDirectories.end()) {
                ignoreDirectories.push_back(directory);
            }
        }

        void Watcher::removeIgnoreDirectory(const std::string& directory) {
            std::lock_guard<std::mutex> lock(directoriesMutex);
            
            auto it = std::find(ignoreDirectories.begin(), ignoreDirectories.end(), directory);
            if (it != ignoreDirectories.end()) {
                ignoreDirectories.erase(it);
            }
        }

        void Watcher::addToRenderQueue(const std::filesystem::path& path) {
            std::lock_guard<std::mutex> lock(renderQueueMutex);
            if (!incomingRenderQueue.contains(path)) {
                incomingRenderQueue.push(path);
            }
        }

        void Watcher::addToDeleteQueue(const std::filesystem::path& path) {
            std::lock_guard<std::mutex> lock(deleteQueueMutex);
            if (!incomingDeleteQueue.contains(path)) {
                incomingDeleteQueue.push(path);
            }
        }

        void Watcher::watcherThreadFunc() {
            // Create a new FileWatcher
            fileWatcher = std::make_unique<efsw::FileWatcher>(false);
            
            // Create a new UpdateListener
            updateListener = std::make_unique<UpdateListener>(this);
            
            // Configure the watcher
            fileWatcher->followSymlinks(false);
            fileWatcher->allowOutOfScopeLinks(false);
            
            // Check if the directory exists
            if (!watchPath.empty() && std::filesystem::exists(watchPath)) {
                // Add the directory to watch
                efsw::WatchID watchID = fileWatcher->addWatch(watchPath, updateListener.get(), true);
                
                if (watchID > 0) {
                    // Start watching
                    fileWatcher->watch();
                    std::cout << "Watching directory: " << watchPath << std::endl;
                    
                    // Run until stopped
                    while (!stopRequested) {
                        efsw::System::sleep(500);
                    }
                } else {
                    std::cout << "Error trying to watch directory: " << watchPath << std::endl;
                    std::cout << efsw::Errors::Log::getLastErrorLog().c_str() << std::endl;
                }
            } else {
                std::cout << "Error: Directory does not exist: " << watchPath << std::endl;
            }
        }

    } // namespace file
} // namespace oom
