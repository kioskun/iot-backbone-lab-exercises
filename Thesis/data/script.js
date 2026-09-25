// script.js
console.log("[boot] script.js loaded");
// --- GLOBAL CHART VARIABLES --- //
let myChartDHTTemp = null;
let myChartDHTHum = null;
let myChartPhoto = null;

// Arrays for the DHT sensor
let dhtLabels = [];   // Timestamps for temperature
let tempData = [];    // Temperature readings
let humData = [];     // Humidity readings

// Photo arrays
let photoLabels = [];
let lightData = [];

// Polling timer handle
let pollTimer = null;

/**
 * Show a toast notification
 * @param {string} message - Message to display in the toast
 */
function showToast(message) {
  const toast = document.getElementById("toast");
  toast.innerText = "🔔 " + message;
  toast.classList.add("show");

  const bell = document.getElementById("bell");
  if (bell) bell.classList.add("ring");

  setTimeout(() => {
    toast.classList.remove("show");
    if (bell) bell.classList.remove("ring");
  }, 3000);
}

/**
 * Initialize the application when the window loads
 */
window.onload = function() {
  console.log("[boot] window.onload fired");
  
  loadPage('/control_panel.partial.html');
  
  // Add event listener for mobile menu toggle
  const menuToggle = document.createElement('div');
  menuToggle.className = 'menu-toggle';
  menuToggle.innerHTML = '☰';
  menuToggle.addEventListener('click', () => {
    document.querySelector('.nav-menu').classList.toggle('active');
  });
  
  // Only add the toggle for mobile view
  if (window.innerWidth <= 768) {
    document.querySelector('header').insertBefore(
      menuToggle, 
      document.querySelector('.main-nav')
    );
  }
};

/**
 * Load a partial HTML page with fade transition
 * @param {string} url - URL of the partial HTML to load
 */
function loadPage(url) {
  console.log("[nav] fetching", url);

  let contentDiv = document.getElementById("content");
  contentDiv.style.opacity = 0;
  
  setTimeout(() => {
    fetch(url)
      .then(response => response.text())
      .then(html => {
		console.log("[nav] fetched", url, "ok, length:", html.length);

        contentDiv.innerHTML = html;
        contentDiv.style.opacity = 1;

        // If the wifi_setup partial loaded, fetch current creds
        if (url.includes('wifi_setup.partial.html')) {
          fetchCurrentWifiCreds();
        }

        // If the control panel partial has loaded, set up everything
        if (url.includes('control_panel')) {
          // 1) Fetch toggles (active/inactive)
          fetchSensorStatus();

          // 2) Create the DHT Temp/Hum charts and Photo chart after HTML is inserted
          setTimeout(() => {
            let dhtTempCanvas = document.getElementById('dhtTempChart');
            let dhtHumCanvas = document.getElementById('dhtHumChart');
            let photoCanvas = document.getElementById('photoChart');
            
            if (dhtTempCanvas) createDHTTempChart();
            if (dhtHumCanvas) createDHTHumChart();
            if (photoCanvas) createPhotoChart();

            // 3) Start polling for sensor values
            startPollingSensorValues();
          }, 300);
        }
      })
      .catch(err => {
		console.error("[nav] error loading", url, err);
        console.error("Error loading page: " + err);
        showToast("Error loading page");
        contentDiv.style.opacity = 1;
      });
  }, 300);
}

function fetchCurrentWifiCreds() {
  fetch('/getWifiCreds')
    .then(res => res.json())
    .then(data => {
      const ssidSpan = document.getElementById('currentSsid');
      const passSpan = document.getElementById('currentPass');
      if (ssidSpan && passSpan) {
        ssidSpan.textContent = data.ssid || "";
        passSpan.textContent = data.password || "";
      }
    })
    .catch(err => {
      console.error("Error fetching WiFi creds", err);
      showToast("Error fetching Wi-Fi credentials");
    });
}

/**
 * Create the DHT Temperature Chart
 */
function createDHTTempChart() {
  if (typeof Chart === "undefined") { console.warn("Chart.js missing; skipping DHT Temp chart"); return; }
  let ctx = document.getElementById('dhtTempChart').getContext('2d');
  myChartDHTTemp = new Chart(ctx, {
    type: 'line',
    data: {
      labels: dhtLabels,
      datasets: [
        {
          label: 'Temperature (°C)',
          data: tempData,
          borderColor: '#e74c3c',
          backgroundColor: 'rgba(231, 76, 60, 0.1)',
          borderWidth: 2,
          fill: true,
          tension: 0.3
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          min: 0,
          max: 50,
          ticks: {
            stepSize: 5
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.05)'
          }
        },
        x: {
          grid: {
            color: 'rgba(0, 0, 0, 0.05)'
          }
        }
      },
      plugins: {
        legend: {
          labels: {
            color: '#333'
          }
        }
      },
      interaction: {
        mode: 'index',
        intersect: false
      }
    }
  });
}

/**
 * Create the DHT Humidity Chart
 */
function createDHTHumChart() {
  if (typeof Chart === "undefined") { console.warn("Chart.js missing; skipping DHT Hum chart"); return; }
  let ctx = document.getElementById('dhtHumChart').getContext('2d');
  myChartDHTHum = new Chart(ctx, {
    type: 'line',
    data: {
      labels: dhtLabels,
      datasets: [
        {
          label: 'Humidity (%)',
          data: humData,
          borderColor: '#3498db',
          backgroundColor: 'rgba(52, 152, 219, 0.1)',
          borderWidth: 2,
          fill: true,
          tension: 0.3
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          min: 0,
          max: 100,
          ticks: {
            stepSize: 10
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.05)'
          }
        },
        x: {
          grid: {
            color: 'rgba(0, 0, 0, 0.05)'
          }
        }
      },
      plugins: {
        legend: {
          labels: {
            color: '#333'
          }
        }
      },
      interaction: {
        mode: 'index',
        intersect: false
      }
    }
  });
}

/**
 * Create the Photoresistor Light Level Chart
 */
function createPhotoChart() {
  if (typeof Chart === "undefined") { console.warn("Chart.js missing; skipping Photo chart"); return; }
  let ctx = document.getElementById('photoChart').getContext('2d');
  myChartPhoto = new Chart(ctx, {
    type: 'line',
    data: {
      labels: photoLabels,
      datasets: [
        {
          label: 'Light Level',
          data: lightData,
          borderColor: '#f39c12',
          backgroundColor: 'rgba(243, 156, 18, 0.1)',
          borderWidth: 2,
          fill: true,
          tension: 0.3
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          min: 0,
          max: 4095,
          ticks: {
            stepSize: 500
          },
          grid: {
            color: 'rgba(0, 0, 0, 0.05)'
          }
        },
        x: {
          grid: {
            color: 'rgba(0, 0, 0, 0.05)'
          }
        }
      },
      plugins: {
        legend: {
          labels: {
            color: '#333'
          }
        }
      },
      interaction: {
        mode: 'index',
        intersect: false
      }
    }
  });
}

/**
 * Fetch sensor status and update UI toggles
 */
function fetchSensorStatus() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      if (data.DHT !== undefined) {
        let dhtToggle = document.getElementById("DHTToggle");
        if (dhtToggle) {
          dhtToggle.checked = data.DHT;
          updateToggleUI("DHT", data.DHT);
        }
      }
      if (data.Photoresistor !== undefined) {
        let photoToggle = document.getElementById("PhotoresistorToggle");
        if (photoToggle) {
          photoToggle.checked = data.Photoresistor;
          updateToggleUI("Photoresistor", data.Photoresistor);
        }
      }
    })
    .catch(err => {
      console.error("Error fetching sensor status: " + err);
      showToast("Error fetching sensor status");
    });
}

/**
 * Update the UI elements for a sensor toggle
 * @param {string} sensorName - Name of the sensor
 * @param {boolean} isActive - Whether the sensor is active
 */
function updateToggleUI(sensorName, isActive) {
  let circle = document.getElementById(sensorName + "Circle");
  let statusText = document.getElementById(sensorName + "Status");
  
  if (circle) {
    circle.className = isActive ? "status-circle active" : "status-circle inactive";
  }
  if (statusText) {
    statusText.innerText = isActive ? "Active" : "Inactive";
  }
  
  // Fade+slide the sensor card body
  if (sensorName === "DHT") {
    toggleFadeSlide("DHTCardBody", isActive);
    toggleFadeSlide("dhtTempChartContainer", isActive);
    toggleFadeSlide("dhtHumChartContainer", isActive);
  } else if (sensorName === "Photoresistor") {
    toggleFadeSlide("PhotoCardBody", isActive);
    toggleFadeSlide("photoChartContainer", isActive);
  }
}

/**
 * Toggle a sensor's active state
 * @param {string} sensorName - Name of the sensor to toggle
 * @param {boolean} isChecked - New state of the toggle
 */
function toggleSensor(sensorName, isChecked) {
  let action = isChecked ? 'enable' : 'disable';
  
  fetch(`/sensor?sensor=${sensorName}&action=${action}`)
    .then(response => response.text())
    .then(data => {
      console.log(data);
      showToast(data);
      
      setTimeout(() => {
        fetchSensorStatus();
      }, 500);
    })
    .catch(err => {
      console.error(err);
      showToast("Error toggling sensor");
    });
}

/**
 * Start polling for sensor values
 */
function startPollingSensorValues() {
  if (pollTimer) clearInterval(pollTimer);
  pollTimer = setInterval(pollSensorValues, 3000);
}

/**
 * Format a time value in seconds to HH:MM:SS
 */
 
function formatTime(seconds) {
  const hrs = Math.floor(seconds / 3600);
  const mins = Math.floor((seconds % 3600) / 60);
  const secs = seconds % 60;
  return `${hrs.toString().padStart(2, '0')}:${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
}


/**
 * Poll sensor values from the server
 */
function pollSensorValues() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      // Update DHT sensor data if active
      if (data.DHT) {
        let now = new Date().toLocaleTimeString();
        dhtLabels.push(now);
        
        tempData.push(data.DHT_Temp !== undefined ? data.DHT_Temp : null);
        humData.push(data.DHT_Hum !== undefined ? data.DHT_Hum : null);
        
        // Keep only the last 20 data points
        if (dhtLabels.length > 20) {
          dhtLabels.shift();
          tempData.shift();
          humData.shift();
        }
        
        // Update charts if they exist
        if (myChartDHTTemp) myChartDHTTemp.update();
        if (myChartDHTHum) myChartDHTHum.update();
        
        // Update current values in the UI
        const tempValue = document.getElementById('currentTemp');
        const humValue = document.getElementById('currentHum');
        
        if (tempValue && data.DHT_Temp !== undefined) {
          tempValue.textContent = `${data.DHT_Temp.toFixed(1)}°C`;
        }
        
        if (humValue && data.DHT_Hum !== undefined) {
          humValue.textContent = `${data.DHT_Hum.toFixed(1)}%`;
        }
      }
      
      // Update Photoresistor data if active
      if (data.Photoresistor) {
        let now = new Date().toLocaleTimeString();
        photoLabels.push(now);
        
        lightData.push(data.Photo !== undefined ? data.Photo : null);
        
        // Keep only the last 20 data points
        if (photoLabels.length > 20) {
          photoLabels.shift();
          lightData.shift();
        }
        
        // Update chart if it exists
        if (myChartPhoto) myChartPhoto.update();
        
        // Update current values in the UI
        const lightValue = document.getElementById('currentLight');
        const ledState = document.getElementById('ledState');
        
        if (lightValue && data.Photo !== undefined) {
          lightValue.textContent = data.Photo;
        }
        
        if (ledState && data.LED !== undefined) {
          ledState.textContent = data.LED ? "ON" : "OFF";
          ledState.className = data.LED ? "led-on" : "led-off";
        }
      }
	  if (data.uptime !== undefined) {
		document.getElementById("uptime").textContent = formatTime(parseInt(data.uptime));
	  }
	  if (data.lastTx !== undefined) {
		document.getElementById("lastTx").textContent = formatTime(parseInt(data.lastTx));
	  }
    })
    .catch(err => {
      console.error("Error fetching sensor values: " + err);
      showToast("Error fetching sensor data");
    });
}

function toggleFadeSlide(elementId, show) {
  const el = document.getElementById(elementId);
  if (!el) return;
  
  if (show) {
    el.classList.remove('hide-panel');
    el.classList.add('show-panel');
  } else {
    el.classList.remove('show-panel');
    el.classList.add('hide-panel');
  }
}

function deployWifiSettings() {
  const ssid = document.getElementById("newSsid").value;
  const pass = document.getElementById("newPass").value;

  if (!ssid) {
    showToast("SSID cannot be empty");
    return;
  }

  // Confirm the user is okay with a reboot
  const ok = confirm("Deploy new Wi-Fi settings and reboot?");
  if (!ok) return;

  // We do a POST with the new credentials
  fetch("/deployWifi", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(pass)}`
  })
    .then(res => res.text())
    .then(txt => {
      console.log(txt);
      showToast(txt);
      // The ESP will reboot ~500ms after responding, 
      // so we won't see much more happen on this page
    })
    .catch(err => {
      console.error(err);
      showToast("Error deploying new Wi-Fi credentials");
    });
}
