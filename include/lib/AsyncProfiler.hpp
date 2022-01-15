#pragma once
#include "main.h"

/**
 * @brief an enum that contains all the possible states for our motion profile controller
 * 
 */
enum class MotionProfileState{
    MOVE, FOLLOW, IDLE
};

// forward declare
template class StateMachine<MotionProfileState>;

/**
 * @brief class that allows us to control our chassis asynchronously using motion profiles
 * 
 */
class AsyncMotionProfiler : public StateMachine<MotionProfileState, MotionProfileState::IDLE>, public TaskWrapper{
    protected:
    /**
     * @brief Construct a new Async Motion Profiler object. The constructor is declared to be protected to make sure
     *        that this can only be constructed by its friend profiler builder class
     * 
     * @param iChassis the chassis to output the power to
     * @param iMove the motion profile generator for linear movements
     * @param iLeftLinear the left motor controller for linaer movements
     * @param iRightLinear the right motor controller for linear movements
     * @param iLeftTrajectory the left motor controller for following trajectories
     * @param iRightTrajectory the left motor controller for following trajectories
     * @param iTimeUtil timer for the object
     */
    AsyncMotionProfiler(std::shared_ptr<ChassisController> iChassis, 
                        std::unique_ptr<LinearMotionProfile> iMove, 
                        std::unique_ptr<FFVelocityController> iLeftLinear, 
                        std::unique_ptr<FFVelocityController> iRightLinear,
                        std::unique_ptr<FFVelocityController> iLeftTrajectory,
                        std::unique_ptr<FFVelocityController> iRightTrajectory,
                        const TimeUtil& iTimeUtil);

    /**
     * @brief copying is not allowed
     */
    void operator=(const AsyncMotionProfiler& rhs) = delete;

    /**
     * @brief forward declared as friend to allow AsyncMotionProfilerBuilder to access the constructor
     * 
     */
    friend class AsyncMotionProfilerBuilder;

    std::shared_ptr<ChassisController> chassis;
    std::shared_ptr<AbstractMotor> leftMotor;
    std::shared_ptr<AbstractMotor> rightMotor;

    std::unique_ptr<LinearMotionProfile> profiler;
    std::unique_ptr<FFVelocityController> leftLinear;
    std::unique_ptr<FFVelocityController> rightLinear;
    std::unique_ptr<FFVelocityController> leftTrajectory;
    std::unique_ptr<FFVelocityController> rightTrajectory;

    TimeUtil timeUtil;
    std::unique_ptr<AbstractRate> rate;
    std::unique_ptr<AbstractTimer> timer;
    QTime maxTime{0.0};

    Trajectory path;
    pros::Mutex lock;

    public:
    /**
     * @brief Set the target distance to travel linearly
     * 
     * @param iDistance 
     */
    void setTarget(QLength iDistance);

    /**
     * @brief Set the target trajectory for the robot to follow
     * 
     * @param iPath the trajectory to follow
     */
    void setTarget(const Trajectory& iPath);

    /**
     * @brief stops the chassis movement
     * 
     */
    void stop();

    /**
     * @brief Task loop to run for the chassis to be controlled asynchronously
     * 
     */
    void loop() override;

    /**
     * @brief Blocks the most recent movement until it is complete
     * 
     */
    void waitUntilSettled();
};

/**
 * @brief since constructing an AsyncMotionProfiler requires a lot of parameter that can be confusing to users, 
 *        we wrote this builder class which allows easier / more straight forward instantiation of the class
 * 
 */
class AsyncMotionProfilerBuilder{
    public:
    /**
     * @brief Construct a new Async Motion Profiler Builder object
     * 
     */
    AsyncMotionProfilerBuilder();

    /**
     * @brief Destroy the Async Motion Profiler Builder object
     * 
     */
    ~AsyncMotionProfilerBuilder() = default;

    /**
     * @brief sets the chassis object for the profiler to output to
     * 
     * @param iChassis the chassis object to output to
     * @return AsyncMotionProfilerBuilder& an ongoing builder
     */
    AsyncMotionProfilerBuilder& withOutput(std::shared_ptr<ChassisController> iChassis);

    /**
     * @brief sets the motion profile generator to use for linear movements
     * 
     * @param iProfiler the profile generator
     * @return AsyncMotionProfilerBuilder& 
     */
    AsyncMotionProfilerBuilder& withProfiler(std::unique_ptr<LinearMotionProfile> iProfiler);

    /**
     * @brief sets the motor controller to use for linear movements
     * 
     * @param iLeft 
     * @param iRight 
     * @return AsyncMotionProfilerBuilder& 
     */
    AsyncMotionProfilerBuilder& withLinearController(FFVelocityController iLeft, FFVelocityController iRight);

    /**
     * @brief sets the motor controller to use when following paths    
     * 
     * @param iLeft 
     * @param iRight 
     * @return AsyncMotionProfilerBuilder& 
     */
    AsyncMotionProfilerBuilder& withTrajectoryController(FFVelocityController iLeft, FFVelocityController iRight);

    /**
     * @brief builds the async motion profiler object with the specified parameters. The thread is started automaically
     * 
     * @return std::shared_ptr<AsyncMotionProfiler> the built async motion profiler
     */
    std::shared_ptr<AsyncMotionProfiler> build();

    private:
    std::unique_ptr<LinearMotionProfile> profile;
    std::shared_ptr<ChassisController> chassis;
    FFVelocityController leftL;
    FFVelocityController rightL;
    FFVelocityController leftT;
    FFVelocityController rightT;

    bool linearInit = false;
    bool trajInit = false;
    bool driveInit = false;
    bool profileInit = false;
};