import { createInertiaApp } from '@inertiajs/svelte'

createInertiaApp({
  resolve: name => require(`@/pages/${name}.svelte`),
  setup({ el, App, props }) {
    new App({ target: el, props })
  },
})