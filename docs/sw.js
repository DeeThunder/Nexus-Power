const CACHE_NAME = 'nexus-power-v3';
const ASSETS = [
  './',
  './index.html',
  './dashboard.html',
  './manifest.json',
  './devices.json',
  './assets/css/styles.css',
  './assets/images/icon-512.png',
  './assets/images/hero.png',
  './assets/images/hardware.png'
];

// Install: cache all shell assets
self.addEventListener('install', event => {
  event.waitUntil(
    caches.open(CACHE_NAME).then(cache => cache.addAll(ASSETS))
  );
  self.skipWaiting();
});

// Activate: clean up old caches
self.addEventListener('activate', event => {
  event.waitUntil(
    caches.keys().then(keys =>
      Promise.all(keys.filter(k => k !== CACHE_NAME).map(k => caches.delete(k)))
    )
  );
  self.clients.claim();
});

// Fetch: serve from cache, fall back to network
self.addEventListener('fetch', event => {
  event.respondWith(
    caches.match(event.request).then(cached => cached || fetch(event.request))
  );
});
