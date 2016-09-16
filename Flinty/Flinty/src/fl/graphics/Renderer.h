#pragma once

#include "fl/Common.h"

#include <queue>

namespace fl {

	class FL_API RenderCommand
	{
	public:
		virtual void OnExecute() = 0;
	};

	class FL_API Renderer
	{
	private:
		static Renderer* s_Instance;
	private:
		std::queue<RenderCommand*> m_CommandBuffer;
	public:
		Renderer();
		static void Init();

		void Submit(RenderCommand* command);

		void Clear();
		void SetClearColor(float r, float g, float b, float a);
		void Flush();

		inline static Renderer* GetInstance() { FL_ASSERT(s_Instance);  return s_Instance; }
	};

}

#define SUBMIT_RENDER_0(x) class Command##__LINE__ : public RenderCommand { public: void OnExecute() override { x } }; fl::Renderer::GetInstance()->Submit(new Command ## __LINE__ ())
#define SUBMIT_RENDER_1(param0Type, param0Name, x) class Command ## __LINE__ : public RenderCommand { private: param0Type param0; public: Command##__LINE__(param0Type& param0) : param0Name(param0) void OnExecute() override { x } }; fl::Renderer::GetInstance()->Submit(new Command ## __LINE__ (param0Name))
#define SUBMIT_RENDER_2(param0Type, param0Name, param1Type, param1Name, x) class Command ## __LINE__ : public RenderCommand { private: param0Type param0; param1Type param1; public: Command ## __LINE__ (param0Type param0, param1Type param1) : param0(param0), param1(param1) {} void OnExecute() override { x } }; fl::Renderer::GetInstance()->Submit(new Command ## __LINE__ (param0Name, param1Name))
#define SUBMIT_RENDER_3(param0Type, param0Name, param0Value, param1Type, param1Name, param1Value, param2Type, param2Name, param2Value, x) \
			class Command ## __LINE__ : public RenderCommand\
			{\
			private:\
				param0Type param0Name;\
				param1Type param1Name;\
				param2Type param2Name;\
			public:\
				Command ## __LINE__ (param0Type param0, param1Type param1, param2Type param2)\
					: param0Name(param0), param1Name(param1), param2Name(param2) {}\
				void OnExecute() override { x }\
			};\
			fl::Renderer::GetInstance()->Submit(new Command ## __LINE__ (param0Value, param1Value, param2Value))

