import {defineConfig} from "vite";
import vue from "@vitejs/plugin-vue";

export default defineConfig(({command}) => ({
  plugins: [vue()],
  base: command === 'build' ? '/kamisado-awase/' : '/',
  server: {
    port: 5173,
    // proxy: {
    //   '/api': {
    //     target: 'http://localhost:8081',
    //     changeOrigin: true,
    //   },
    //   '/ws': {
    //     target: 'ws://localhost:8081',
    //     changeOrigin: true,
    //     ws: true
    //   },
    // }
  }
}));
