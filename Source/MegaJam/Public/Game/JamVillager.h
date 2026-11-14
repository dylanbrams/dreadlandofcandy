#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamVillager.generated.h"

UCLASS()
class MEGAJAM_API AJamVillager : public AActor
{
	GENERATED_BODY()
public:
	AJamVillager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void SetName(const FString& Name);
	void SetRole(FName Giver);
private:
	UPROPERTY() TObjectPtr<class UStaticMeshComponent> Art;
	UPROPERTY() TObjectPtr<class UStaticMeshComponent> RoleProp;
	UPROPERTY() TObjectPtr<class UTextRenderComponent> Label;
};
