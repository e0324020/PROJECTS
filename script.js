/**
 * LYRIC PIANO: REWRITE THE STARS (Chorus Celestial Edition)
 * Sound: Shimmering, prolonging, and magical.
 * Logic: Full Chorus Syllables - One key per note.
 */

// --- 1. THE CELESTIAL ENGINE ---
const reverb = new Tone.Reverb({ decay: 10, wet: 0.6 }).toDestination();

const celestial = new Tone.PolySynth(Tone.FMSynth, {
    harmonicity: 3,
    modulationIndex: 10,
    oscillator: { type: "sine" },
    envelope: { 
        attack: 0.1, 
        decay: 0.2, 
        sustain: 1, 
        release: 5.0 // The "prolonging" magic
    }
}).connect(reverb);

// --- 2. THE COMPLETE CHORUS MAP ---
const songMap = [
    { word: "WHAT",    letter: "W", note: "Bb3" },
    { word: "IF",      letter: "I", note: "C4" },
    { word: "WE",      letter: "W", note: "Bb3" },
    { word: "RE-",     letter: "R", note: "Bb4" },
    { word: "-WRITE",  letter: "W", note: "A4" },
    { word: "THE",     letter: "T", note: "F4" },
    { word: "STARS?",  letter: "S", note: "Eb4" },
    
    { word: "SAY",     letter: "S", note: "Bb3" },
    { word: "YOU",     letter: "Y", note: "C4" },
    { word: "WERE",    letter: "W", note: "Bb3" },
    { word: "MADE",    letter: "M", note: "G4" },
    { word: "TO",      letter: "T", note: "F4" },
    { word: "BE",      letter: "B", note: "Bb3" },
    { word: "MINE",    letter: "M", note: "C4" },

    { word: "NO-",     letter: "N", note: "Bb3" },
    { word: "THING",   letter: "T", note: "C4" },
    { word: "COULD",   letter: "C", note: "Bb3" },
    { word: "KEEP",    letter: "K", note: "Bb4" },
    { word: "US",      letter: "U", note: "A4" },
    { word: "A-",      letter: "A", note: "F4" },
    { word: "-PART",   letter: "P", note: "Eb4" },

    { word: "YOU'D",   letter: "Y", note: "Bb3" },
    { word: "BE",      letter: "B", note: "C4" },
    { word: "THE",     letter: "T", note: "Bb3" },
    { word: "ONE",     letter: "O", note: "F4" },
    { word: "I",       letter: "I", note: "Bb3" },
    { word: "WAS",     letter: "W", note: "Bb3" },
    { word: "MEANT",   letter: "M", note: "F4" },
    { word: "TO",      letter: "T", note: "Bb3" },
    { word: "FIND",    letter: "F", note: "C4" }
];

// --- 3. CORE LOGIC ---
let pointer = 0;
let isStarted = false;
let currentActiveKey = null;

const uiWord = document.getElementById('active-word');
const uiLetter = document.getElementById('target-letter');
const uiNext = document.getElementById('next-line');

async function startGame() {
    console.log("Celestial Chorus Starting...");
    await Tone.start();
    document.getElementById('menu-screen').style.display = 'none';
    document.getElementById('game-board').style.display = 'block';
    isStarted = true;
    updateUI();
}

document.addEventListener('keydown', (e) => {
    if (!isStarted || pointer >= songMap.length || e.repeat) return;
    const goal = songMap[pointer];
    if (e.key.toLowerCase() === goal.letter.toLowerCase()) {
        currentActiveKey = e.key.toLowerCase();
        celestial.triggerAttack(goal.note);
        uiWord.classList.add('holding');
    }
});

document.addEventListener('keyup', (e) => {
    if (!isStarted || !currentActiveKey) return;
    if (e.key.toLowerCase() === currentActiveKey) {
        celestial.triggerRelease(songMap[pointer].note);
        uiWord.classList.remove('holding');
        currentActiveKey = null;
        pointer++;
        updateUI();
    }
});

function updateUI() {
    if (pointer >= songMap.length) {
        uiWord.innerText = "STARDUST!";
        uiLetter.innerText = "✨";
        uiNext.innerText = "The Chorus is complete.";
        return;
    }
    const current = songMap[pointer];
    const next = songMap[pointer + 1];
    uiWord.innerText = current.word;
    uiLetter.innerText = current.letter;
    uiNext.innerText = next ? "Next: " + next.word : "The finale...";
}