#include "PAEnemySpawner.h"
#include "PABotCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

APAEnemySpawner::APAEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SpawnRadius = 1000.0f;
	SpawnRate = 2.0f;
	MaxEnemies = 10;
	bSpawnEnabled = true;
	
	TimeSinceLastSpawn = 0.0f;
	CurrentEnemyCount = 0;
}

void APAEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
}

void APAEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bSpawnEnabled && EnemyClass)
	{
		TimeSinceLastSpawn += DeltaTime;
		
		if (TimeSinceLastSpawn >= 1.0f / SpawnRate && CurrentEnemyCount < MaxEnemies)
		{
			SpawnEnemy();
			TimeSinceLastSpawn = 0.0f;
		}
	}
}

void APAEnemySpawner::SpawnEnemy()
{
	if (EnemyClass)
	{
		FVector SpawnLocation = GetRandomSpawnLocation();
		FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		APABotCharacter* SpawnedEnemy = GetWorld()->SpawnActor<APABotCharacter>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
		
		if (SpawnedEnemy)
		{
			CurrentEnemyCount++;
			
			// 当敌人死亡时减少计数
			SpawnedEnemy->OnDestroyed.AddDynamic(this, &APAEnemySpawner::OnEnemyDestroyed);
		}
	}
}

void APAEnemySpawner::OnEnemyDestroyed(AActor* DestroyedActor)
{
	CurrentEnemyCount--;
}

FVector APAEnemySpawner::GetRandomSpawnLocation()
{
	AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerActor)
	{
		FVector PlayerLocation = PlayerActor->GetActorLocation();
		
		// 在玩家周围的圆环上生成敌人
		float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
		FVector Direction(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f);
		FVector SpawnLocation = PlayerLocation + Direction * SpawnRadius;
		
		// 确保生成位置在可导航区域内
		UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (NavSystem)
		{
			FNavLocation NavLocation;
			if (NavSystem->ProjectPointToNavigation(SpawnLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f)))
			{
				return NavLocation.Location;
			}
		}
		
		return SpawnLocation;
	}
	
	return GetActorLocation();
}