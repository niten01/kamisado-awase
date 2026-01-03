import {createApp} from 'vue'
import App from './App.vue'
import Toast, {POSITION} from "vue-toastification";
import "vue-toastification/dist/index.css";
import './style.css'


createApp(App)
  .use(Toast, {
    position: POSITION.TOP_RIGHT,
    containerClassName: "kami-toast-container",
    toastClassName: "kami-toast",
    bodyClassName: "kami-toast-body",

    newestOnTop: true,
    maxToasts: 5,
    timeout: 2800,

    transition: "Vue-Toastification__slideBlurred",

    showCloseButtonOnHover: true
  })
  .mount("#app");

