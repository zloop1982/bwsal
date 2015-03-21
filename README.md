# bwsal
Automatically exported from code.google.com/p/bwsal

The BWAPI Standard Add-on Library (BWSAL) is a project that aims to develop several add-ons for BWAPI that will be useful for a wide variety of AIs,.


Optimal micro strategies for two opposing units
Let A and B denote two opposing units in a real-time strategy game where each unit has 5 fixed properties: speed, range, dps (damage per shot), and cooldown. Each unit also has 2 dynamic properties: position (a vector in Rn) and health, a scalar. We also assume that the initial distance between both units is greater than the range of either unit. That is, |A.position-B.position| > max(A.range,B.range).

Let shots(X,Y) denote the number of shots it takes for X to kill Y. Since each shot by X reduces Y's health by X.dps, we can define shots(X,Y) as follows:

shots(X,Y) = ceil(Y.health/X.dps)

Let timeToKill(X,Y) denote the time it takes for X to kill Y, assuming |X.position-Y.position|<X.range for the entire time:

timeToKill(X,Y) = (shots(X,Y)-1)*X.cooldown

A.range can be less than, equal to, or greater than B.range, and A.speed can be less than, equal to, or greater than B.speed, so there are 9 possible cases when considering both of them which we will group into the following 4 cases:

A.speed > B.speed and A.range > B.range
A.speed > B.speed and A.range <= B.range
A.speed = B.speed
A.speed < B.speed
A.speed > B.speed and A.range > B.range
In this case, A can always win regardless of the other unit properties. Since A.speed > B.speed, it can control whether |A.position-B.position| increases, decreases, or stays the same at each point in time. Furthermore, since A.range > B.range, A can move toward B until A.range > |A.position-B.position| > B.range, and then maintain that distance while it attacks B. At this distance B cannot attack A, and since B is slower than A, it cannot get close enough to attack A nor evade A, so A always wins. This is commonly called "kiting".

A.speed > B.speed and A.range <= B.range
In this case, A can move faster than B, but it does not have a greater range. However A can still kill B if the time it takes for A to kill B plus the time it takes for A to move from a distance of |A.position-B.position| = B.range to a distance of |A.position-B.position| = A.range is less than the time it takes for B to kill A. The time it takes for A to move a distance of B.range - A.range is (B.range-A.range)/A.speed, however we should assume that B will try to maximize the amount of time it can attack A without A being able to attack B, so we use the time (B.range-A.range)/(A.speed-B.speed). Our condition can then be written as:

timeToKill(A,B) + (B.range-A.range)/(A.speed-B.speed) < timeToKill(B,A)

If this inequality is true, A can attack and kill B.

If not, A may still be able to kill B, not in a straight up attack, but by quickly micro'ing in and out of range of B. This strategy only makes sense to consider if the following to pre-condition holds: A's cooldown is longer than the time needed to move out of B's attack range and back in. If this precondition is false, then A will not have time to micro out of range of B and back in before it needs to fire its next shot, and so microing in and out of range will just reduce A's attack rate. This precondition can be stated as the following inequality:

A.cooldown > 2*(B.range-A.range)/(A.speed-B.speed)

Assuming this precondition holds, we can say that the number of shots B can get off during each time that A is in B's range is:

shotsPerMicroRound(B,A) = ceil((2*(B.range-A.range)/(A.speed-B.speed))/B.cooldown)

where "MicroRound" is the event of A moving into range of B to attack B and then moving back out.

Similarly, the number of full micro rounds for B to kill A is:

fullMicroRounds(B,A) = floor(shots(B,A) / shotsPerMicroRound(B,A))

The last micro round is not full and has the following number of remaining shots:

remainingShots(B,A) = shots(B,A) % shotsPerMicroRound(B,A)

From this we can express the time it takes B to kill A, given that A is microing in and out of range:

adjustedTimeToKill(B,A) = fullMicroRounds(B,A) * A.cooldown + remainingShots(B,A)* B.cooldown

From this, our condition becomes:

timeToKill(A,B) + (B.range-A.range)/(A.speed-B.speed) < adjustedTimeToKill(B,A)

If this inequality is true, then A can attack and kill B.

A.speed = B.speed
In this case neither unit can win if they are both rational - the weaker unit will always retreat, and since they can move at the same speed, the stronger unit will never be able to attack. Nonetheless, A should try to attack iff A.range > B.range or A.range = B.range and timeToKill(A,B) < timeToKill(B,A) . If B.range > A.range, B should be trying to attack A and A should retreat.

A.speed < B.speed
If B is rational, this case has only 2 outcomes - B chooses to attack and kill A, or B evades A. The decision making for B proceeds as it did for A in the first 2 cases. If A.range >= B.range, then A will be able to do damage to B so it should try to do as much damage to B before it dies.

Heuristics for m units vs n units
While micro'ing m units of group A vs n units of group B is NP-hard, one possible heuristic for approximately solving this would be to have group A micro so as to maximize timeToKill'(B,A) while not increasing timeToKill'(A,B), where timeToKill'(X,Y) is a heuristic generalization of timeToKill(X,Y) that works for two groups of units that are all assumed to be in range of each other.

Computing timeToKill'(X,Y):

Let X be a set of m units that all have the same speed, range, dps, and cooldown, but possibly different health.

Let Y be a set of n units that all have the same speed, range, dps, and cooldown, but possibly different health.

Let Y.healthi denote the health of unit i in set Y.

Then we can say the number of shots for a unit in X to kill Yj is :

shotsG(X, Yj) = ceil(Y.healthj / X.dps)

From this, the number of shots for a unit in X to kill all of Y is:

shotsG(X, Y) = sum(shotsG(X, Yj), j=1 to m)

Since X has m units that can all attack at once, the time it takes for X to kill Y is:

timeToKill'(X,Y) = (ceil(shotsG(X, Y)/m) - 1) * X.cooldown
