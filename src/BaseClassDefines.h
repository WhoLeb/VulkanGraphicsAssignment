#pragma once

#define NO_COPY(className) className(const className&) = delete;\
	className& operator=(const className&) = delete

#define NO_COPY_NO_MOVE(className) className(const className&) = delete;\
	className& operator=(const className&) = delete;\
	className(className&&) = delete;\
	className& operator=(const className&&) = delete
