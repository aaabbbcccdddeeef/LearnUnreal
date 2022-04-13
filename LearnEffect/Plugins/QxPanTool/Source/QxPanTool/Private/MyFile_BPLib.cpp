// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFile_BPLib.h"
#include <Factories/CurveFactory.h>
#include <Engine/Texture2D.h>
#include <AssetRegistry/AssetRegistryModule.h>
#include <Misc/PackageName.h>
#include "FileHelpers.h"
#include <Editor/EditorEngine.h>
#include <Containers/UnrealString.h>
#include <Factories/TextureFactory.h>

FReply UMyFile_BPLib::CreateTestTexture()
{
	
	FString  AssetRelativePath = TEXT("Test");
	FString packageName = TEXT("/Game");
	packageName = packageName / AssetRelativePath;
	packageName = packageName + TEXT("/");

	FString assetNAME = TEXT("TestTexture");
	packageName += assetNAME;

	UPackage* testPacakge = CreatePackage(nullptr, *packageName);
	//curFactory->FactoryCreateNew(UCurveFloat::StaticClass(), nullptr, "Test", EObjectFlags);
	testPacakge->FullyLoad();


	UTexture2D* NewTexture = NewObject<UTexture2D>(testPacakge,
		*assetNAME, RF_Public | RF_Standalone | RF_MarkAsRootSet);

	int32  TextureWidth = 800;
	int32 TextureHeight = 600;
	NewTexture->AddToRoot();
	NewTexture->PlatformData = new FTexturePlatformData();
	NewTexture->PlatformData->SizeX = TextureWidth;
	NewTexture->PlatformData->SizeY = TextureHeight;
	//NewTexture->PlatformData->NumSlices = 1;
	NewTexture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

	//myTextAsset->
	uint8* Pixels = new uint8[TextureWidth * TextureHeight * 4];
	for (int32 y = 0; y < TextureHeight; y++)
	{
		for (int32 x = 0; x < TextureWidth; x++)
		{
			int32 curPixelIndex = (y * TextureWidth) + x;
			Pixels[4 * curPixelIndex] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 1] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 2] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 3] = 255;
		}
	}

	FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips)FTexture2DMipMap(); //Node *q = new (p)Node(10); 就是从p为首地址分配一段内存给q
	Mip->SizeX = TextureWidth;
	Mip->SizeY = TextureHeight;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = (uint8*)Mip->BulkData.Realloc(TextureWidth * TextureHeight * 4);
	FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * TextureWidth * TextureHeight * 4);

	Mip->BulkData.Unlock();

	NewTexture->UpdateResource();
	testPacakge->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);
	
	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		packageName, FPackageName::GetAssetPackageExtension());
	/*bool bSaved = UPackage::SavePackage(testPacakge, NewTexture,
		EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
		*PackageFileName, GError, nullptr, true, true, SAVE_NoError);*/

	//bool res = UPackage::SavePackage(testPacakge, NewTexture, EObjectFlags::RF_Public | RF_Standalone, *PackageFileName);
	//check(res);

	//if (GEditor)
	//{
	//	GEditor->SavePackage(testPacakge, NewTexture, EObjectFlags::RF_Public | RF_Standalone, *PackageFileName);
	//}


	const FString PackageName = testPacakge->GetName();

	// Place were we should save the file, including the filename
	//FString FinalPackageSavePath;
	//// Just the filename
	//FString FinalPackageFilename;

	//// True if we should attempt saving
	//bool bAttemptSave = false;

	//// If the package already has a valid path to a non read-only location, use it to determine where the file should be saved
	//const bool bIncludeReadOnlyRoots = false;
	//const bool bIsValidPath = FPackageName::IsValidLongPackageName(PackageName, bIncludeReadOnlyRoots);
	//const bool bPackageAlreadyExists = FPackageName::DoesPackageExist(PackageName, NULL, &ExistingFilename);
	//check(bPackageAlreadyExists)

	FString testPackageName = testPacakge->GetName();
	FString ExistingFilename;
	FPackageName::DoesPackageExist(testPackageName, NULL, &ExistingFilename);


	FString FinalPackageFilename = FString::Printf(TEXT("%s%s"), *testPacakge->GetName(), *FPackageName::GetAssetPackageExtension());
	//FinalPackageFilename = FPackageName::LongPackageNameToFilename(SaveAsPackageName);
	GEngine->Exec(NULL,
		*FString::Printf(
			TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"),
			*testPackageName, *PackageFileName), *GError);

	delete[] Pixels;

	return FReply::Handled();
}

FReply UMyFile_BPLib::CreateTestTexture2()
{
	FString  AssetRelativePath = TEXT("Test");
	FString packageName = TEXT("/Game");
	packageName = packageName / AssetRelativePath;
	packageName = packageName + TEXT("/");

	FString assetNAME = TEXT("TestTexture2");
	packageName += assetNAME;

	UPackage* testPacakge = CreatePackage(nullptr, *packageName);
	//curFactory->FactoryCreateNew(UCurveFloat::StaticClass(), nullptr, "Test", EObjectFlags);
	testPacakge->FullyLoad();


	UTexture2D* NewTexture = NewObject<UTexture2D>(testPacakge,
		*assetNAME, RF_Public | RF_Standalone | RF_MarkAsRootSet);

	int32  TextureWidth = 800;
	int32 TextureHeight = 600;
	NewTexture->AddToRoot();
	NewTexture->PlatformData = new FTexturePlatformData();
	NewTexture->PlatformData->SizeX = TextureWidth;
	NewTexture->PlatformData->SizeY = TextureHeight;
	//NewTexture->PlatformData->NumSlices = 1;
	NewTexture->PlatformData->SetNumSlices(1);
	NewTexture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

	//myTextAsset->
	uint8* Pixels = new uint8[TextureWidth * TextureHeight * 4];
	for (int32 y = 0; y < TextureHeight; y++)
	{
		for (int32 x = 0; x < TextureWidth; x++)
		{
			int32 curPixelIndex = (y * TextureWidth) + x;
			Pixels[4 * curPixelIndex] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 1] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 2] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 3] = 255;
		}
	}

	FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips)FTexture2DMipMap(); //Node *q = new (p)Node(10); 就是从p为首地址分配一段内存给q
	Mip->SizeX = TextureWidth;
	Mip->SizeY = TextureHeight;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = (uint8*)Mip->BulkData.Realloc(TextureWidth * TextureHeight * 4);
	FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * TextureWidth * TextureHeight * 4);

	Mip->BulkData.Unlock();

	NewTexture->UpdateResource();
	testPacakge->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);

	// Force all mips to stream in, as we may need to use mip 0 for the thumbnail
	NewTexture->SetForceMipLevelsToBeResident(30.0f);
	NewTexture->WaitForStreaming();

	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		packageName, FPackageName::GetAssetPackageExtension());
	/*bool bSaved = UPackage::SavePackage(testPacakge, NewTexture,
		EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
		*PackageFileName, GError, nullptr, true, true, SAVE_NoError);*/

	bool res = UPackage::SavePackage(testPacakge, NewTexture, EObjectFlags::RF_Public | RF_Standalone, *PackageFileName);
	check(res);

	delete[] Pixels;

	//TArray<UPackage*> PackagesToSave;
	//PackagesToSave.Add(testPacakge);
	//FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, /*bPromptToSave=*/ false);
	//
	return FReply::Handled();
}

FReply UMyFile_BPLib::CreateTestTexture3()
{
	
	FString  AssetRelativePath = TEXT("Test");
	FString packageName = TEXT("/Game");
	packageName = packageName / AssetRelativePath;
	packageName = packageName + TEXT("/");

	FString assetNAME = TEXT("TestTexture3");
	packageName += assetNAME;

	UPackage* testPacakge = CreatePackage(nullptr, *packageName);
	//curFactory->FactoryCreateNew(UCurveFloat::StaticClass(), nullptr, "Test", EObjectFlags);
	testPacakge->FullyLoad();

	auto TextureFact = NewObject<UTextureFactory>();
	TextureFact->AddToRoot();

	int32  TextureWidth = 800;
	int32 TextureHeight = 600;
	//myTextAsset->
	uint8* Pixels = new uint8[TextureWidth * TextureHeight * 4];
	for (int32 y = 0; y < TextureHeight; y++)
	{
		for (int32 x = 0; x < TextureWidth; x++)
		{
			int32 curPixelIndex = (y * TextureWidth) + x;
			Pixels[4 * curPixelIndex] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 1] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 2] = FMath::RandHelper(255);
			Pixels[4 * curPixelIndex + 3] = 255;
		}
	}
	const uint8* startPointer = Pixels;
	UTexture2D* NewTexture = NULL;
	NewTexture = (UTexture2D*)TextureFact->FactoryCreateBinary(
		UTexture2D::StaticClass(), testPacakge, *assetNAME,
		RF_Standalone | RF_Public, NULL, TEXT("png"), startPointer,
		startPointer + TextureWidth * TextureHeight * 4,
		GWarn);

	//UTexture2D* NewTexture = NewObject<UTexture2D>(testPacakge,
	//	*assetNAME, RF_Public | RF_Standalone | RF_MarkAsRootSet);

	NewTexture->AddToRoot();
	NewTexture->PlatformData = new FTexturePlatformData();
	NewTexture->PlatformData->SizeX = TextureWidth;
	NewTexture->PlatformData->SizeY = TextureHeight;
	//NewTexture->PlatformData->NumSlices = 1;
	NewTexture->PlatformData->SetNumSlices(1);
	NewTexture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

	

	FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips)FTexture2DMipMap(); //Node *q = new (p)Node(10); 就是从p为首地址分配一段内存给q
	Mip->SizeX = TextureWidth;
	Mip->SizeY = TextureHeight;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = (uint8*)Mip->BulkData.Realloc(TextureWidth * TextureHeight * 4);
	FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * TextureWidth * TextureHeight * 4);

	Mip->BulkData.Unlock();

	NewTexture->UpdateResource();
	testPacakge->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);
	
	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		packageName, FPackageName::GetAssetPackageExtension());


	const FString PackageName = testPacakge->GetName();


	FString testPackageName = testPacakge->GetName();
	FString ExistingFilename;

	FString FinalPackageFilename = FString::Printf(TEXT("%s%s"), *testPacakge->GetName(), *FPackageName::GetAssetPackageExtension());
	GEngine->Exec(NULL,
		*FString::Printf(
			TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"),
			*testPackageName, *PackageFileName), *GError);

	delete[] Pixels;

	return FReply::Handled();
}
