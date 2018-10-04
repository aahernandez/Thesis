#pragma once

const float INTENSITY_THRESHOLD_DIMINISHER = 0.1f;
const float DECAY_SPEED = 0.001f;
const float SOCIAL_RELATION_CHANGE_SPEED = 0.001f;
const float EMOTION_CHANGE_SPEED = 0.01f;

enum BasicEmotion
{
	NO_BASIC_EMOTION = -1,
	JOY,
	HOPE,
	RELIEF,
	PRIDE,
	ADMIRATION,
	DISTRESS,
	FEAR,
	DISAPPOINTMENT,
	SHAME,
	ANGER,
	NUM_BASIC_EMOTIONS,
};

enum PersonalityTrait
{
	OPENNESS,
	CONSCIENTIOUSNESS,
	EXTRAVERSION,
	AGREEABLENESS,
	NEUROTOCISM,
	NUM_PERSONALITY_TRAITS,
};