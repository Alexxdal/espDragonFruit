let isConnected = false;
let updateInterval = null;

const CHIP_MODELS = {
    1: 'ESP32',
    2: 'ESP32-S2',
    5: 'ESP32-C3',
    6: 'ESP32-S3',
    9: 'ESP32-C2',
    12: 'ESP32-C5',
    13: 'ESP32-C6',
    16: 'ESP32-H2',
    17: 'ESP32-P4'
};

const WIFI_MODES = {
    0: 'NULL',
    1: 'STA',
    2: 'AP',
    3: 'APSTA'
};

function formatBytes(bytes) {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return Math.round((bytes / Math.pow(k, i)) * 100) / 100 + ' ' + sizes[i];
}

function formatFeatures(features) {
    const flags = [];
    if (features & 0x01) flags.push('WiFi');
    if (features & 0x02) flags.push('BLE');
    if (features & 0x04) flags.push('BT');
    if (features & 0x08) flags.push('EMB_FLASH');
    if (features & 0x10) flags.push('EMB_PSRAM');
    return flags.join(', ') || 'None';
}

function updateBoardUI(prefix, data) {
    if (!data) {
        document.getElementById(`${prefix}Card`).classList.add('unavailable');
        return;
    }

    document.getElementById(`${prefix}Card`).classList.remove('unavailable');

    if (data.chip) {
        const modelName = CHIP_MODELS[data.chip.model] || `Unknown (${data.chip.model})`;
        const el = document.getElementById(`${prefix}ChipModel`);
        if (el) el.textContent = modelName;
        
        const modelBadge = document.getElementById(`${prefix}Model`);
        if (modelBadge) modelBadge.textContent = modelName;
        
        const coresEl = document.getElementById(`${prefix}Cores`);
        if (coresEl) coresEl.textContent = data.chip.cores || '-';
        
        const revEl = document.getElementById(`${prefix}Revision`);
        if (revEl) revEl.textContent = data.chip.revision || '-';
        
        const featEl = document.getElementById(`${prefix}Features`);
        if (featEl) featEl.textContent = formatFeatures(data.chip.features);
    }

    if (data.ram) {
        const total = data.ram.total_internal;
        const free = data.ram.free_internal;
        const used = total - free;
        const percentage = total > 0 ? (used / total) * 100 : 0;

        const textEl = document.getElementById(`${prefix}RamText`);
        if (textEl) {
            textEl.textContent = `${formatBytes(used)} / ${formatBytes(total)}`;
        }

        const barEl = document.getElementById(`${prefix}RamBar`);
        if (barEl) {
            barEl.style.width = `${percentage}%`;
            if (percentage > 90) {
                barEl.style.background = 'linear-gradient(90deg, #ff1744, #f50057)';
            } else if (percentage > 70) {
                barEl.style.background = 'linear-gradient(90deg, #ffd600, #ff6f00)';
            } else {
                barEl.style.background = 'linear-gradient(90deg, #00e676, #7c4dff)';
            }
        }

        const freeEl = document.getElementById(`${prefix}RamFree`);
        if (freeEl) freeEl.textContent = formatBytes(free);
        
        const largestEl = document.getElementById(`${prefix}RamLargest`);
        if (largestEl) largestEl.textContent = formatBytes(data.ram.largest_contig);
        
        const spiramEl = document.getElementById(`${prefix}Spiram`);
        if (spiramEl) {
            spiramEl.textContent = data.ram.spiram_size > 0 ? formatBytes(data.ram.spiram_size) : 'No';
        }
    }

    if (data.modules) {
        updateModuleBadge(`${prefix}Spi`, data.modules.spi);
        updateModuleBadge(`${prefix}Netif`, data.modules.netif);
        updateModuleBadge(`${prefix}WifiInit`, data.modules.wifi_init);
        updateModuleBadge(`${prefix}WifiStarted`, data.modules.wifi_started);
        updateModuleBadge(`${prefix}Bluetooth`, data.modules.bluetooth);
    }

    if (data.wifi) {
        const ssidEl = document.getElementById(`${prefix}ApSsid`) || document.getElementById(`${prefix}Ssid`);
        if (ssidEl) {
            const ssid = data.wifi.ap_ssid || '-';
            ssidEl.textContent = ssid;
        }

        const channelEl = document.getElementById(`${prefix}Channel`);
        if (channelEl) channelEl.textContent = data.wifi.ap_channel || '-';
        
        const modeEl = document.getElementById(`${prefix}Mode`);
        if (modeEl) {
            const mode = WIFI_MODES[data.wifi.mode] || 'Unknown';
            modeEl.textContent = mode;
        }
    }
}

function updateModuleBadge(elementId, isActive) {
    const el = document.getElementById(elementId);
    if (!el) return;
    
    el.classList.remove('active', 'inactive');
    if (isActive) {
        el.classList.add('active');
    } else {
        el.classList.add('inactive');
    }
}

async function fetchStatus() {
    try {
        const response = await fetch('/status');
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        
        const data = await response.json();
        
        if (!isConnected) {
            isConnected = true;
            document.getElementById('connectionStatus').classList.add('connected');
        }

        updateBoardUI('master', data.master);
        updateBoardUI('slave1', data.slave1);
        updateBoardUI('slave2', data.slave2);
        updateBoardUI('slave3', data.slave3);

        const now = new Date();
        const timeStr = now.toLocaleTimeString();
        document.getElementById('lastUpdate').textContent = `Last update: ${timeStr}`;

    } catch (error) {
        console.error('Error fetching status:', error);
        if (isConnected) {
            isConnected = false;
            document.getElementById('connectionStatus').classList.remove('connected');
            document.getElementById('lastUpdate').textContent = 'Connection lost';
        }
    }
}

function startAutoUpdate() {
    fetchStatus();
    
    if (updateInterval) {
        clearInterval(updateInterval);
    }
    
    updateInterval = setInterval(fetchStatus, 1000);
}

document.addEventListener('DOMContentLoaded', () => {
    console.log('espDragonFruit Dashboard initialized');
    startAutoUpdate();
});

window.addEventListener('beforeunload', () => {
    if (updateInterval) {
        clearInterval(updateInterval);
    }
});
