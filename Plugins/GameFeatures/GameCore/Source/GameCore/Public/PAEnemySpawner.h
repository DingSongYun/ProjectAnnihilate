#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PAEnemySpawner.generated.h"

UCLASS()
class GAMECORE_API APAEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:    
	APAEnemySpawner();
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<class APABotCharacter> EnemyClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 MaxEnemies;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bSpawnEnabled;
	
protected:
	virtual void BeginPlay() override;
	
	void SpawnEnemy();
	FVector GetRandomSpawnLocation();
	void OnEnemyDestroyed(AActor* DestroyedActor);
	
	float TimeSinceLastSpawn;
	int32 CurrentEnemyCount;
};