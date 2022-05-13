/*!*******************************************************************************************
 *  \file       follow_path_base.hpp
 *  \brief      follow_path_base header file
 *  \authors    Miguel Fernández Cortizas
 *              Pedro Arias Pérez
 *              David Pérez Saura
 *              Rafael Pérez Seguí
 *
 *  \copyright  Copyright (c) 2022 Universidad Politécnica de Madrid
 *              All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

#ifndef FOLLOW_PATH_BASE_HPP
#define FOLLOW_PATH_BASE_HPP

#include "as2_core/node.hpp"
#include "as2_core/names/actions.hpp"
#include "as2_core/names/topics.hpp"

#include "rclcpp_action/rclcpp_action.hpp"

#include <as2_msgs/action/follow_path.hpp>
#include <nav_msgs/msg/odometry.hpp>

#include <Eigen/Dense>
#include <deque>

namespace follow_path_base
{
    class FollowPathBase
    {
    public:
        using GoalHandleFollowPath = rclcpp_action::ServerGoalHandle<as2_msgs::action::FollowPath>;

        void initialize(as2::Node *node_ptr, float goal_threshold)
        {
            node_ptr_ = node_ptr;
            goal_threshold_ = goal_threshold;
            odom_sub_ = node_ptr_->create_subscription<nav_msgs::msg::Odometry>(
                node_ptr_->generate_global_name(as2_names::topics::self_localization::odom), as2_names::topics::self_localization::qos,
                std::bind(&FollowPathBase::odomCb, this, std::placeholders::_1));

            this->ownInit();
        };

        virtual rclcpp_action::GoalResponse onAccepted(const std::shared_ptr<const as2_msgs::action::FollowPath::Goal> goal) = 0;
        virtual rclcpp_action::CancelResponse onCancel(const std::shared_ptr<GoalHandleFollowPath> goal_handle) = 0;
        virtual bool onExecute(const std::shared_ptr<GoalHandleFollowPath> goal_handle) = 0;

        virtual ~FollowPathBase(){};

    protected:
        FollowPathBase(){};

        // To initialize needed publisher for each plugin
        virtual void ownInit(){};

    private:
        // TODO: if onExecute is done with timer no atomic attributes needed
        void odomCb(const nav_msgs::msg::Odometry::ConstSharedPtr msg)
        {
            this->current_pose_x_ = msg->pose.pose.position.x;
            this->current_pose_y_ = msg->pose.pose.position.y;
            this->current_pose_z_ = msg->pose.pose.position.z;

            this->actual_speed_ = Eigen::Vector3d(msg->twist.twist.linear.x,
                                                  msg->twist.twist.linear.y,
                                                  msg->twist.twist.linear.z)
                                      .norm();
        };

    protected:
        as2::Node *node_ptr_;
        float goal_threshold_;

        std::atomic<float> current_pose_x_;
        std::atomic<float> current_pose_y_;
        std::atomic<float> current_pose_z_;

        std::atomic<float> actual_speed_;

        std::deque<Eigen::Vector3d> waypoints_;

    private:
        rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
    }; // FollowPathBase class

} // follow_path_base namespace

#endif // FOLLOW_PATH_BASE_HPP