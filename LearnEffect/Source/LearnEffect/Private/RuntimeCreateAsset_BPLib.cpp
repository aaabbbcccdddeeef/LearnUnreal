// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeCreateAsset_BPLib.h"
//#include <Factories/CurveFactory.h>
#include <Engine/Texture2D.h>
#include <AssetRegistry/AssetRegistryModule.h>
#include <Misc/PackageName.h>
//#include "FileHelpers.h"
#include <Containers/UnrealString.h>


void URuntimeCreateAsset_BPLib::CreateTestTexture2()
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

	NewTexture->PostEditChange();
	testPacakge->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		packageName, FPackageName::GetAssetPackageExtension());
	/*bool bSaved = UPackage::SavePackage(testPacakge, NewTexture,
		EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
		*PackageFileName, GError, nullptr, true, true, SAVE_NoError);*/

		//bool res = UPackage::SavePackage(testPacakge, NewTexture, EObjectFlags::RF_Public | RF_Standalone, *PackageFileName);
		//check(res);


	delete[] Pixels;

	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(testPacakge);
	//FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, /*bPromptToSave=*/ false);

	FString test1 = testPacakge->GetName();
	FString test2 = testPacakge->GetPathName();
	//FString test3 = testPacakge->get
	FString test4 = testPacakge->GetName();

	FString testPackageName = testPacakge->GetName();

	FString FinalPackageFilename = FString::Printf(TEXT("%s%s"), *testPacakge->GetName(), *FPackageName::GetAssetPackageExtension());
	//FinalPackageFilename = FPackageName::LongPackageNameToFilename(SaveAsPackageName);
	GEngine->Exec(NULL,
		*FString::Printf(
			TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"),
			*testPackageName, *PackageFileName));
}



void URuntimeCreateAsset_BPLib::CreateTestTexture3()
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

	int32 test1 = NewTexture->GetSizeX();
	int32 test2 = NewTexture->GetSizeY();
	NewTexture->UpdateResource();
	testPacakge->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		packageName, FPackageName::GetAssetPackageExtension());
	/*bool bSaved = UPackage::SavePackage(testPacakge, NewTexture,
		EObjectFlags::RF_Public | EObjectFlags::RF_Standalone,
		*PackageFileName, GError, nullptr, true, true, SAVE_NoError);*/

	bool res = UPackage::SavePackage(testPacakge, NewTexture, EObjectFlags::RF_Public | RF_Standalone, *PackageFileName);
		//check(res);

	delete[] Pixels;

	//TArray<UPackage*> PackagesToSave;
	//PackagesToSave.Add(testPacakge);
	////FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, /*bPromptToSave=*/ false);

	//FString test1 = testPacakge->GetName();
	//FString test2 = testPacakge->GetPathName();
	////FString test3 = testPacakge->get
	//FString test4 = testPacakge->GetName();

	//FString testPackageName = testPacakge->GetName();

	//FString FinalPackageFilename = FString::Printf(TEXT("%s%s"), *testPacakge->GetName(), *FPackageName::GetAssetPackageExtension());
	////FinalPackageFilename = FPackageName::LongPackageNameToFilename(SaveAsPackageName);
	//GEngine->Exec(NULL,
	//	*FString::Printf(
	//		TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"),
	//		*testPackageName, *PackageFileName));
}
