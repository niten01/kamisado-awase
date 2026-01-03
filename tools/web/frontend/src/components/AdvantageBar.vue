<script setup>
import {computed} from "vue";

const props = defineProps({
  analysis: {type: Object, required: true},
  side: {type: String, required: true}, // "white" | "black"
});

// clamp and map to [0..100] where 50 is equal
const percentWhite = computed(() => {
  const x = props.analysis.advantageWhite;
  // Clamp roughly; adjust to your evaluation scale:
  const clamped = Math.max(-1, Math.min(1, x));
  return 50 + clamped * 50;
});

const fillStyle = computed(() => {
  // We color by sign using CSS variables, but keep it simple:
  const playerAdvantage = props.analysis.advantageWhite * (props.side === "white" ? 1 : -1);
  const isGood = playerAdvantage >= 0;
  return {
    width: `${100 - percentWhite.value}%`,
  };
});

const label = computed(() => {
  console.log(props.analysis)
  return (props.side === "white" ? props.analysis.formattedScoreWhite : props.analysis.formattedScoreBlack);
});
</script>

<template>
  <div class="adv">
    <div style="display:flex; align-items:center; justify-content:space-between; margin-bottom:10px">
      <div style="font-weight:900">Advantage</div>
      <div class="badge">{{ label }}</div>
    </div>

    <div class="adv-track">
      <div class="adv-mid"></div>
      <div class="adv-fill" :style="fillStyle"></div>
    </div>

    <div class="adv-labels">
      <span>Black</span>
      <span>Equal</span>
      <span>White</span>
    </div>
  </div>
</template>
