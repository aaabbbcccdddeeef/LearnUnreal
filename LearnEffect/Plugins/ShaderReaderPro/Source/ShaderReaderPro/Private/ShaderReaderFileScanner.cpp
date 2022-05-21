// Copyright 2020 Jorge CR. All Rights Reserved.



#include "ShaderReaderFileScanner.h"

FShaderReaderFileScanner::FShaderReaderFileScanner(FOnShaderFileUpdated InOnFileUpdated)
{
	OnFileUpdated = InOnFileUpdated;
	Stopping = false;
	Thread = FRunnableThread::Create(this, TEXT("FFileScanner"), 8 * 1024, TPri_Normal);
}

FShaderReaderFileScanner::~FShaderReaderFileScanner()
{
	if (Thread != nullptr)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

bool FShaderReaderFileScanner::Init()
{
	return true;
}

uint32 FShaderReaderFileScanner::Run()
{
	while (!Stopping)
	{
		for (auto& map : files)
		{
			if (FPaths::FileExists(map.Key))
			{
				FDateTime  TimeStamp = IFileManager::Get().GetStatData(*map.Key).ModificationTime;
				if (TimeStamp > map.Value)
				{
					map.Value = TimeStamp;
					FString File = map.Key;
					AsyncTask(ENamedThreads::GameThread, [this, File]()
					{
						OnFileUpdated.ExecuteIfBound(File);
						UE_LOG(ShaderReaderLog, Verbose, TEXT("File updated"));
					});
				}
			}
		}
		FPlatformProcess::Sleep(3.0f);
	}
	return 1;
}

void FShaderReaderFileScanner::Stop()
{
	Stopping = true;
}

void FShaderReaderFileScanner::Exit()
{

}

void FShaderReaderFileScanner::RegisterFile(const FString& File)
{
	if (!files.Contains(File))
	{
		files.Add(File, FDateTime());
	}
}

void FShaderReaderFileScanner::UpdateImmidiatly(const FString& File)
{
	OnFileUpdated.ExecuteIfBound(File);
	UE_LOG(ShaderReaderLog, Verbose, TEXT("File updated"));
}
