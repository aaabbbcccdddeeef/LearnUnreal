// Copyright 2020 Jorge CR. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include <HAL/Runnable.h>
#include <HAL/RunnableThread.h>

DECLARE_DELEGATE_OneParam(FOnShaderFileUpdated, const FString&);

/**
 * 
 */
class SHADERREADERPRO_API FShaderReaderFileScanner : public FRunnable
{
public:
	FShaderReaderFileScanner(FOnShaderFileUpdated InOnFileUpdated);
	~FShaderReaderFileScanner();

public:
	//~ FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;


private:

	//~ Hold the files to be scanned
	TMap<FString, FDateTime> files;

	/** Holds the thread object. */
	FRunnableThread* Thread;


	bool Stopping;

private:
	//~ Delegate that triggers when the file has changes
	FOnShaderFileUpdated OnFileUpdated;

public:
	/**
	 * Register a file to see if is having changes once in a while
	 *
	 * @param: File - File to be registered by the file scanner
	*/
	void RegisterFile(const FString& File);
	void UpdateImmidiatly(const FString& File);
};
