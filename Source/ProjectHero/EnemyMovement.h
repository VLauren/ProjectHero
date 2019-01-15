#pragma once

#include "CoreMinimal.h"
#include "PHMovement.h"
#include "EnemyMovement.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTHERO_API UEnemyMovement : public UPHMovement
{
	GENERATED_BODY()

private:
	float StartGravity;
	FRotator CurrentRotation;
	FVector MoveVector;
	
protected:
	bool SpLaunch;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

public:
	virtual bool IsGrounded();

	void Launch(float amount, bool spLaunch);

	void AirHit();

	void Move(float DeltaTime, FVector Destination);
	void RotateTowards(float DeltaTime, FVector Destination);
};
