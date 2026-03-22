import { useState, useCallback } from "react";

/* ══════════════════════════════════════════════════════════════════════════
   CrushClone Theme V3
   iOS 26 glassmorphism — palette matched to original CamelCrusher screenshot
   ══════════════════════════════════════════════════════════════════════════
   Color mapping vs. original:
   • Outer frame      → thick amber-gold border  #E2BE6E / #C8973E
   • Inner shell      → dark purple-navy          #1C1A28 / #17161E
   • Section cards    → dark slate panels         #231F30 / #1E1C2A
   • ON buttons       → blue pill                 #4A7CE0 / #6898F0
   • Knob dots/labels → gold                      #C8973E
   • Knob body        → dark charcoal             #2E2A3A
   • Patch LCD        → dark bg + blue text       #7AADFF
   • Phat button      → dark square + blue P
   ══════════════════════════════════════════════════════════════════════════ */

/* ─── Design tokens ──────────────────────────────────────────────────────── */
const T = {
  /* frame gold */
  fgA: "#E2BE6E",
  fgB: "#C8973E",
  fgC: "#9A7A2C",
  fgGlow:     "rgba(200,151,62,0.55)",
  fgGlowSoft: "rgba(200,151,62,0.18)",

  /* shell — dark purple-navy */
  shellA: "#1C1A28",
  shellB: "#17161E",
  shellC: "#100E1A",

  /* cards — dark slate panels */
  cardA: "#231F30",
  cardB: "#1E1C2A",
  cardC: "#191728",
  cardBorder:    "rgba(200,151,62,0.24)",
  cardBorderTop: "rgba(230,190,100,0.42)",
  cardShine:     "rgba(255,240,190,0.07)",
  cardInner:     "rgba(255,230,150,0.03)",

  /* gold accent — knobs, labels, arrows */
  gA: "#F0D070",
  gB: "#C8973E",
  gC: "#8A6820",
  gGlow:     "rgba(200,151,62,0.45)",
  gGlowSoft: "rgba(200,151,62,0.14)",
  gText:     "#C8973E",

  /* ON button — blue */
  onA: "#6898F0",
  onB: "#4A7CE0",
  onC: "#2E55AA",
  onGlow:   "rgba(74,124,224,0.52)",
  onBorder: "rgba(140,180,255,0.55)",

  /* LCD patch display */
  lcdBg:     "#0B0A14",
  lcdText:   "#7AADFF",
  lcdGlow:   "rgba(122,173,255,0.28)",
  lcdBorder: "rgba(74,124,224,0.22)",

  /* Phat button */
  phatBorder:   "rgba(104,152,240,0.65)",
  phatText:     "#6898F0",
  phatTextGlow: "rgba(74,124,224,0.45)",

  /* mint — phat active */
  mintA:    "#5EEDC8",
  mintGlow: "rgba(94,237,200,0.40)",

  /* knob body */
  knobA: "#2E2A3A",
  knobB: "#21202E",
  knobC: "#141220",
  knobDot: "rgba(200,151,62,0.20)",

  /* text */
  txtA: "#D8D4E8",
  txtB: "#8A86A4",
  txtC: "rgba(140,120,80,0.38)",
};

/* ─── Camel logo ─────────────────────────────────────────────────────────── */
function CamelLogo({ size = 30 }) {
  return (
    <svg width={size} height={size} viewBox="0 0 64 64" fill="none">
      <defs>
        <linearGradient id="lg-camel" x1="0" y1="0" x2="1" y2="1">
          <stop offset="0%"   stopColor={T.gA} />
          <stop offset="50%"  stopColor={T.gB} />
          <stop offset="100%" stopColor={T.gC} />
        </linearGradient>
        <filter id="glow-camel">
          <feGaussianBlur stdDeviation="1.2" result="b" />
          <feMerge>
            <feMergeNode in="b" />
            <feMergeNode in="SourceGraphic" />
          </feMerge>
        </filter>
      </defs>
      <path
        d="M16 48 L16 38 Q16 30 20 26 Q24 22 26 16 L26 12 Q27 10 29 11 L29 16
           Q29 20 32 22 Q36 24 38 22 Q40 20 40 16 L40 11 Q41 10 43 12 L42 16
           Q42 22 46 26 Q50 30 50 38 L50 48
           M50 38 Q52 36 54 38 L54 42 Q54 44 52 44 L50 44
           M16 38 Q14 36 12 38 L12 42 Q12 44 14 44 L16 44
           M16 48 L22 48 L22 42  M30 48 L26 48 L26 42
           M36 48 L40 48 L40 42  M50 48 L44 48 L44 42
           M26 14 Q24 12 26 10 Q28 8 30 10  M22 30 Q20 28 22 26"
        stroke="url(#lg-camel)"
        strokeWidth="2.2"
        strokeLinecap="round"
        strokeLinejoin="round"
        fill="none"
        filter="url(#glow-camel)"
        opacity="0.95"
      />
      <circle cx="28" cy="13" r="1.2" fill={T.gA} opacity="0.9" />
    </svg>
  );
}

/* ─── Rotary Knob ────────────────────────────────────────────────────────── */
function Knob({ label, value, onChange, min = 0, max = 1, size = 68 }) {
  const [drag, setDrag] = useState(false);
  const norm   = (value - min) / (max - min);
  const startA = -135, endA = 135;
  const angle  = startA + norm * (endA - startA);
  const r      = size / 2 - 7;
  const cx = size / 2, cy = size / 2;

  const pol = (a, rad) => ({
    x: cx + rad * Math.cos((a - 90) * Math.PI / 180),
    y: cy + rad * Math.sin((a - 90) * Math.PI / 180),
  });

  const ts = pol(startA, r), te = pol(endA, r), tv = pol(angle, r);
  const bigV   = angle - startA > 180 ? 1 : 0;
  const trackD = `M${ts.x} ${ts.y}A${r} ${r} 0 1 1 ${te.x} ${te.y}`;
  const valD   = norm > 0.005 ? `M${ts.x} ${ts.y}A${r} ${r} 0 ${bigV} 1 ${tv.x} ${tv.y}` : null;
  const pEnd   = pol(angle, r * 0.50);
  const pStart = pol(angle, r * 0.14);

  const onDown = useCallback((e) => {
    e.preventDefault();
    setDrag(true);
    const st = { y: e.clientY, v: value };
    const mv = (e2) => {
      const d = (st.y - e2.clientY) / 150;
      onChange(Math.min(max, Math.max(min, st.v + d * (max - min))));
    };
    const up = () => {
      setDrag(false);
      document.removeEventListener("mousemove", mv);
      document.removeEventListener("mouseup", up);
    };
    document.addEventListener("mousemove", mv);
    document.addEventListener("mouseup", up);
  }, [value, min, max, onChange]);

  return (
    <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: 6, userSelect: "none" }}>
      <div style={{ position: "relative", width: size, height: size }}>
        {/* ambient glow halo */}
        <div style={{
          position: "absolute", inset: -5, borderRadius: "50%",
          background: `radial-gradient(circle, ${T.gGlow} 0%, transparent 72%)`,
          opacity: drag ? 0.6 : 0.15,
          transition: "opacity 0.3s",
          pointerEvents: "none",
        }} />

        <svg
          width={size} height={size}
          style={{
            cursor: "ns-resize",
            position: "relative",
            zIndex: 1,
            filter: drag ? `drop-shadow(0 0 7px ${T.gGlow})` : "none",
          }}
          onMouseDown={onDown}
        >
          <defs>
            <linearGradient id="arc-grad" x1="0%" y1="0%" x2="100%" y2="100%">
              <stop offset="0%"   stopColor={T.gA} />
              <stop offset="55%"  stopColor={T.gB} />
              <stop offset="100%" stopColor={T.gC} stopOpacity="0.7" />
            </linearGradient>
            <radialGradient id="knob-sphere" cx="36%" cy="30%" r="64%">
              <stop offset="0%"   stopColor={T.knobA} />
              <stop offset="45%"  stopColor={T.knobB} />
              <stop offset="100%" stopColor={T.knobC} />
            </radialGradient>
            <radialGradient id="knob-spec" cx="33%" cy="27%" r="36%">
              <stop offset="0%"   stopColor="rgba(255,255,255,0.16)" />
              <stop offset="100%" stopColor="rgba(255,255,255,0)" />
            </radialGradient>
          </defs>

          {/* background track */}
          <path d={trackD} fill="none" stroke="rgba(200,151,62,0.08)" strokeWidth="2.5" strokeLinecap="round" />

          {/* 11 gold dot markers */}
          {Array.from({ length: 11 }, (_, i) => {
            const a  = startA + (i / 10) * (endA - startA);
            const p  = pol(a, r + 4);
            const on = i / 10 <= norm;
            return (
              <circle
                key={i} cx={p.x} cy={p.y} r="1.8"
                fill={on ? T.gB : T.knobDot}
                opacity={on ? 0.90 : 0.55}
              />
            );
          })}

          {/* sphere body */}
          <circle cx={cx} cy={cy} r={r * 0.54} fill="url(#knob-sphere)" />
          {/* gold rim */}
          <circle cx={cx} cy={cy} r={r * 0.54} fill="none" stroke={T.gC} strokeWidth="0.8" strokeOpacity="0.30" />
          {/* specular highlight */}
          <circle cx={cx} cy={cy} r={r * 0.54} fill="url(#knob-spec)" />

          {/* indicator line */}
          <line
            x1={pStart.x} y1={pStart.y} x2={pEnd.x} y2={pEnd.y}
            stroke={T.gA} strokeWidth="2.5" strokeLinecap="round" opacity="0.92"
          />
          {/* indicator tip */}
          <circle
            cx={pEnd.x} cy={pEnd.y} r="2.5" fill={T.gA}
            style={{ filter: `drop-shadow(0 0 4px ${T.gGlow})` }}
          />
        </svg>
      </div>

      <span style={{
        fontSize: 8.5, fontWeight: 600, letterSpacing: 2.2,
        color: T.gText, textTransform: "uppercase",
        fontFamily: "'DM Mono', monospace",
        textShadow: `0 0 8px ${T.gGlowSoft}`,
      }}>{label}</span>
    </div>
  );
}

/* ─── ON Toggle — blue pill ──────────────────────────────────────────────── */
function OnToggle({ active, onChange }) {
  return (
    <button
      onClick={() => onChange(!active)}
      style={{
        background: active
          ? `linear-gradient(135deg, ${T.onA} 0%, ${T.onB} 60%, ${T.onC} 100%)`
          : "rgba(30,26,44,0.75)",
        border: active
          ? `1px solid ${T.onBorder}`
          : "1px solid rgba(100,90,140,0.28)",
        borderRadius: 6,
        padding: "4px 10px 4px 8px",
        fontSize: 8, fontWeight: 700, letterSpacing: 1.8,
        color: active ? "#FFFFFF" : "rgba(120,110,160,0.50)",
        cursor: "pointer",
        transition: "all 0.22s cubic-bezier(0.34,1.56,0.64,1)",
        boxShadow: active
          ? `0 0 14px ${T.onGlow}, 0 1px 5px rgba(0,0,0,0.55), inset 0 1px 0 rgba(200,220,255,0.22)`
          : "inset 0 1px 0 rgba(255,255,255,0.04)",
        fontFamily: "'DM Mono', monospace",
        display: "flex", alignItems: "center", gap: 5,
        backdropFilter: "blur(6px)",
        textShadow: active ? `0 0 8px ${T.onA}` : "none",
      }}
    >
      <span style={{
        width: 4, height: 4, borderRadius: "50%",
        background: active ? "rgba(220,235,255,0.9)" : "rgba(100,90,140,0.35)",
        display: "inline-block", flexShrink: 0,
        boxShadow: active ? `0 0 5px ${T.onA}` : "none",
      }} />
      ON
    </button>
  );
}

/* ─── Bento Card — dark purple-navy glass panel ──────────────────────────── */
function BentoCard({ title, enabled, onToggle, children }) {
  return (
    <div style={{
      position: "relative",
      background: `linear-gradient(148deg, ${T.cardA} 0%, ${T.cardB} 50%, ${T.cardC} 100%)`,
      backdropFilter: "blur(22px) saturate(1.35)",
      WebkitBackdropFilter: "blur(22px) saturate(1.35)",
      border: `1px solid ${T.cardBorder}`,
      borderTop: `1px solid ${T.cardBorderTop}`,
      borderRadius: 16,
      padding: "13px 16px 18px",
      opacity: enabled ? 1 : 0.30,
      transition: "opacity 0.3s ease, box-shadow 0.3s ease",
      boxShadow: enabled
        ? `0 8px 28px rgba(0,0,0,0.65),
           0 1px 0 rgba(220,185,100,0.06) inset,
           0 -1px 0 rgba(0,0,0,0.45) inset,
           0 0 30px ${T.fgGlowSoft}`
        : "0 4px 12px rgba(0,0,0,0.45)",
      display: "flex", flexDirection: "column", overflow: "hidden",
    }}>
      {/* top specular line */}
      <div style={{
        position: "absolute", top: 0, left: "8%", right: "8%", height: 1,
        background: `linear-gradient(90deg, transparent, ${T.cardShine}, transparent)`,
        pointerEvents: "none",
      }} />
      {/* bottom gold hairline */}
      <div style={{
        position: "absolute", bottom: 0, left: "18%", right: "18%", height: 1,
        background: `linear-gradient(90deg, transparent, ${T.gC}55, transparent)`,
        pointerEvents: "none",
      }} />
      {/* inner glass texture */}
      <div style={{
        position: "absolute", inset: 0, borderRadius: 16,
        background: `radial-gradient(ellipse 75% 40% at 50% 0%, ${T.cardInner}, transparent)`,
        pointerEvents: "none",
      }} />

      <div style={{ display: "flex", alignItems: "center", gap: 9, marginBottom: 13, position: "relative" }}>
        <OnToggle active={enabled} onChange={onToggle} />
        <span style={{
          fontSize: 11, fontWeight: 600, letterSpacing: 1.3,
          color: enabled ? T.txtA : T.txtB,
          transition: "color 0.3s",
          fontFamily: "'DM Sans', sans-serif",
        }}>{title}</span>
      </div>

      <div style={{
        display: "flex", alignItems: "flex-start",
        justifyContent: "center", gap: 18, flex: 1, position: "relative",
      }}>
        {children}
      </div>
    </div>
  );
}

/* ─── Patch Selector ─────────────────────────────────────────────────────── */
const PRESETS = [
  "DRM BigBadBeat", "BAS FatSub", "GTR Crunch", "SYN WarmPad",
  "VOX Gritty", "MAS Loudener", "DRM LoFiHit", "SYN Screamer",
];

function PatchSelector({ index, setIndex }) {
  const prev = () => setIndex((index - 1 + PRESETS.length) % PRESETS.length);
  const next = () => setIndex((index + 1) % PRESETS.length);
  const rand = () => setIndex(Math.floor(Math.random() * PRESETS.length));

  const ArrBtn = ({ dir, fn }) => {
    const [hov, setHov] = useState(false);
    return (
      <button
        onClick={fn}
        onMouseEnter={() => setHov(true)}
        onMouseLeave={() => setHov(false)}
        style={{
          background: hov
            ? `linear-gradient(135deg, ${T.gA} 0%, ${T.gB} 100%)`
            : `linear-gradient(135deg, ${T.gB}DD 0%, ${T.gC}DD 100%)`,
          border: `1px solid ${hov ? T.gA + "88" : T.gC + "88"}`,
          borderRadius: 6,
          color: hov ? "#0C0A14" : "#181208",
          cursor: "pointer",
          width: 26, height: 26,
          display: "flex", alignItems: "center", justifyContent: "center",
          fontSize: 12, padding: 0, fontWeight: 700,
          transition: "all 0.16s ease", flexShrink: 0,
          boxShadow: hov ? `0 0 10px ${T.gGlow}` : "0 1px 3px rgba(0,0,0,0.4)",
        }}
      >{dir === "l" ? "◂" : "▸"}</button>
    );
  };

  return (
    <div style={{
      display: "flex", alignItems: "center", gap: 8,
      background: `linear-gradient(135deg, ${T.cardA}F0 0%, ${T.cardC}F8 100%)`,
      backdropFilter: "blur(18px)",
      WebkitBackdropFilter: "blur(18px)",
      border: `1px solid ${T.cardBorder}`,
      borderTop: `1px solid ${T.cardBorderTop}`,
      borderRadius: 12, padding: "8px 10px",
      boxShadow: `0 4px 16px rgba(0,0,0,0.55), 0 0 22px ${T.fgGlowSoft}, inset 0 1px 0 rgba(200,165,70,0.07)`,
      position: "relative", overflow: "hidden",
    }}>
      {/* top specular */}
      <div style={{
        position: "absolute", top: 0, left: "4%", right: "4%", height: 1,
        background: "linear-gradient(90deg,transparent,rgba(210,175,80,0.22),transparent)",
      }} />

      <span style={{
        fontSize: 7.5, color: T.txtC, fontWeight: 600,
        letterSpacing: 2, textTransform: "uppercase",
        fontFamily: "'DM Mono', monospace", flexShrink: 0,
      }}>Patch</span>

      <ArrBtn dir="l" fn={prev} />

      {/* LCD display */}
      <div style={{
        flex: 1, background: T.lcdBg,
        border: `1px solid ${T.lcdBorder}`,
        borderRadius: 6, padding: "5px 10px", textAlign: "center",
        boxShadow: `inset 0 2px 5px rgba(0,0,0,0.65), 0 0 14px ${T.lcdGlow}`,
      }}>
        <span style={{
          fontSize: 12, fontWeight: 500, display: "block",
          color: T.lcdText, fontFamily: "'DM Mono', monospace",
          letterSpacing: 0.5, whiteSpace: "nowrap",
          overflow: "hidden", textOverflow: "ellipsis",
          textShadow: `0 0 12px ${T.lcdGlow}`,
        }}>{PRESETS[index]}</span>
      </div>

      <ArrBtn dir="r" fn={next} />

      <button
        onClick={rand}
        style={{
          background: `linear-gradient(135deg, ${T.gA} 0%, ${T.gB} 55%, ${T.gC} 100%)`,
          border: `1px solid ${T.gA}55`,
          borderRadius: 6, padding: "5px 11px",
          fontSize: 7.5, fontWeight: 700, letterSpacing: 1.4,
          color: "#0C0A14", cursor: "pointer",
          textTransform: "uppercase", flexShrink: 0,
          transition: "all 0.16s ease",
          boxShadow: `0 0 10px ${T.fgGlowSoft}, 0 2px 5px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,245,195,0.32)`,
          fontFamily: "'DM Mono', monospace",
          textShadow: "0 1px 0 rgba(255,240,180,0.3)",
        }}
        onMouseEnter={(e) => {
          e.currentTarget.style.boxShadow = `0 0 18px ${T.gGlow}, 0 2px 5px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,245,195,0.40)`;
        }}
        onMouseLeave={(e) => {
          e.currentTarget.style.boxShadow = `0 0 10px ${T.fgGlowSoft}, 0 2px 5px rgba(0,0,0,0.5), inset 0 1px 0 rgba(255,245,195,0.32)`;
        }}
      >Randomize</button>
    </div>
  );
}

/* ─── Phat Mode button ───────────────────────────────────────────────────── */
function PhatButton({ active, onChange }) {
  return (
    <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: 6, paddingTop: 4 }}>
      <div style={{ position: "relative" }}>
        {active && (
          <div style={{
            position: "absolute", inset: -8, borderRadius: "50%",
            background: `radial-gradient(circle, ${T.mintGlow} 0%, transparent 70%)`,
            animation: "pulse-glow 2s ease-in-out infinite",
            pointerEvents: "none",
          }} />
        )}
        <div
          onClick={() => onChange(!active)}
          style={{
            position: "relative",
            width: 42, height: 42, borderRadius: 10,
            background: active
              ? `linear-gradient(145deg, rgba(94,237,200,0.22) 0%, ${T.mintA}CC 100%)`
              : `linear-gradient(145deg, ${T.cardA} 0%, ${T.cardC} 100%)`,
            display: "flex", alignItems: "center", justifyContent: "center",
            cursor: "pointer",
            transition: "all 0.28s cubic-bezier(0.34,1.56,0.64,1)",
            boxShadow: active
              ? `0 0 16px ${T.mintGlow}, inset 0 1px 3px rgba(255,255,255,0.16), inset 0 -1px 2px rgba(0,50,35,0.3)`
              : `inset 0 2px 4px rgba(0,0,0,0.55), inset 0 -1px 2px rgba(255,255,255,0.03), 0 1px 5px rgba(0,0,0,0.4)`,
            border: active
              ? "1.5px solid rgba(120,255,210,0.55)"
              : `1.5px solid ${T.phatBorder}`,
            backdropFilter: "blur(8px)",
          }}
        >
          {/* specular */}
          <div style={{
            position: "absolute", top: 4, left: 6, width: 11, height: 6,
            borderRadius: "50%",
            background: active ? "rgba(255,255,255,0.20)" : "rgba(104,152,240,0.12)",
            filter: "blur(2px)", pointerEvents: "none",
          }} />
          <span style={{
            fontSize: 14, fontWeight: 700, position: "relative",
            color: active ? "#002A1A" : T.phatText,
            fontFamily: "'DM Mono', monospace",
            textShadow: active ? "0 1px 2px rgba(0,50,30,0.4)" : `0 0 10px ${T.phatTextGlow}`,
          }}>P</span>
        </div>
      </div>

      <div style={{ textAlign: "center" }}>
        <div style={{
          fontSize: 7.5, fontWeight: 600, letterSpacing: 2,
          color: active ? T.mintA : T.txtB,
          textTransform: "uppercase", transition: "color 0.3s",
          lineHeight: 1.4, fontFamily: "'DM Mono', monospace",
          textShadow: active ? `0 0 10px ${T.mintGlow}` : "none",
        }}>Phat</div>
        <div style={{
          fontSize: 7, fontWeight: 500, letterSpacing: 2,
          color: active ? "rgba(94,237,200,0.70)" : T.txtB,
          textTransform: "uppercase", transition: "color 0.3s",
          fontFamily: "'DM Mono', monospace",
        }}>Mode</div>
      </div>
    </div>
  );
}

/* ─── Main component ─────────────────────────────────────────────────────── */
export default function CrushCloneThemeV3() {
  const [patch,    setPatch]    = useState(0);
  const [tube,     setTube]     = useState(0.35);
  const [mech,     setMech]     = useState(0);
  const [distOn,   setDistOn]   = useState(true);
  const [cutoff,   setCutoff]   = useState(14000);
  const [reso,     setReso]     = useState(0.15);
  const [filterOn, setFilterOn] = useState(true);
  const [compAmt,  setCompAmt]  = useState(0.25);
  const [phat,     setPhat]     = useState(false);
  const [compOn,   setCompOn]   = useState(true);
  const [masterOn, setMasterOn] = useState(true);
  const [volume,   setVolume]   = useState(0);
  const [mix,      setMix]      = useState(1);

  return (
    <div style={{ position: "relative", width: 452, fontFamily: "'DM Sans', -apple-system, sans-serif" }}>

      {/* ambient background glow */}
      <div style={{
        position: "absolute", inset: -20, borderRadius: 42,
        background: `radial-gradient(ellipse 88% 72% at 50% 50%, ${T.fgGlowSoft} 0%, transparent 72%)`,
        pointerEvents: "none", zIndex: 0,
      }} />

      {/* ── Gold frame ──────────────────────────────────────────────────── */}
      <div style={{
        position: "relative", zIndex: 1,
        borderRadius: 22, padding: 5,
        background: `linear-gradient(148deg,
          ${T.fgA}  0%,
          ${T.fgB}  22%,
          ${T.fgC}  48%,
          ${T.fgB}  74%,
          ${T.fgA}  100%)`,
        boxShadow: `
          0 0 0 1px ${T.fgC}CC,
          0 0 28px ${T.fgGlow},
          0 0 55px ${T.fgGlowSoft},
          0 28px 70px rgba(0,0,0,0.80),
          inset 0 1px 0 ${T.fgA}99,
          inset 0 -1px 0 ${T.fgC}88
        `,
      }}>

        {/* corner screw accents */}
        {[{ top: 2, left: 2 }, { top: 2, right: 2 }, { bottom: 2, left: 2 }, { bottom: 2, right: 2 }].map((pos, i) => (
          <div key={i} style={{
            position: "absolute", ...pos,
            width: 10, height: 10, borderRadius: "50%",
            background: `radial-gradient(circle at 35% 30%, ${T.fgA}, ${T.fgC})`,
            boxShadow: `0 0 4px ${T.fgGlow}`, zIndex: 2,
          }} />
        ))}

        {/* ── Inner shell — dark purple-navy ───────────────────────────── */}
        <div style={{
          background: `linear-gradient(155deg, ${T.shellA} 0%, ${T.shellB} 45%, ${T.shellC} 100%)`,
          backdropFilter: "blur(28px) saturate(1.45)",
          WebkitBackdropFilter: "blur(28px) saturate(1.45)",
          borderRadius: 18,
          overflow: "hidden",
          boxShadow: "inset 0 2px 8px rgba(0,0,0,0.55), inset 0 -1px 0 rgba(200,151,62,0.07)",
        }}>
          <div style={{ padding: "20px 20px 22px" }}>

            {/* Header */}
            <div style={{ display: "flex", alignItems: "center", justifyContent: "center", gap: 13, marginBottom: 16 }}>
              <CamelLogo size={33} />
              <div>
                <div style={{ display: "flex", alignItems: "baseline", gap: 5 }}>
                  <span style={{
                    fontSize: 15, fontWeight: 300, color: T.txtB,
                    letterSpacing: 2, fontFamily: "'DM Sans', sans-serif",
                  }}>Crush</span>
                  <span style={{
                    fontSize: 26, fontWeight: 700, letterSpacing: 0.4,
                    background: `linear-gradient(135deg, ${T.gA} 0%, ${T.gB} 60%, ${T.gC} 100%)`,
                    WebkitBackgroundClip: "text", WebkitTextFillColor: "transparent",
                    backgroundClip: "text",
                    fontFamily: "'DM Sans', sans-serif",
                    filter: `drop-shadow(0 0 10px ${T.gGlow})`,
                  }}>Clone</span>
                  <span style={{
                    fontSize: 9, fontWeight: 400, color: T.txtC, letterSpacing: 1.4,
                    fontFamily: "'DM Mono', monospace", marginLeft: 2,
                    alignSelf: "flex-end", paddingBottom: 4,
                  }}>v3.0</span>
                </div>
                <div style={{
                  fontSize: 7.5, color: T.txtC, letterSpacing: 3.5,
                  textTransform: "uppercase", marginTop: 1,
                  fontFamily: "'DM Mono', monospace",
                }}>Multi-effect processor</div>
              </div>
            </div>

            {/* Patch selector */}
            <div style={{ marginBottom: 13 }}>
              <PatchSelector index={patch} setIndex={setPatch} />
            </div>

            {/* 2×2 Bento grid */}
            <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 10 }}>
              <BentoCard title="Distortion" enabled={distOn} onToggle={setDistOn}>
                <Knob label="Tube"   value={tube}   onChange={setTube}   min={0}  max={1} />
                <Knob label="Mech"   value={mech}   onChange={setMech}   min={0}  max={1} />
              </BentoCard>

              <BentoCard title="Filter" enabled={filterOn} onToggle={setFilterOn}>
                <Knob label="Cutoff" value={cutoff} onChange={setCutoff} min={20} max={20000} />
                <Knob label="Res"    value={reso}   onChange={setReso}   min={0}  max={1} />
              </BentoCard>

              <BentoCard title="Compressor" enabled={compOn} onToggle={setCompOn}>
                <Knob label="Amount" value={compAmt} onChange={setCompAmt} min={0} max={1} />
                <PhatButton active={phat} onChange={setPhat} />
              </BentoCard>

              <BentoCard title="Master" enabled={masterOn} onToggle={setMasterOn}>
                <Knob label="Volume" value={volume} onChange={setVolume} min={-24} max={12} />
                <Knob label="Mix"    value={mix}    onChange={setMix}    min={0}   max={1} />
              </BentoCard>
            </div>

            {/* Footer */}
            <div style={{ marginTop: 13, display: "flex", justifyContent: "center", alignItems: "center", gap: 6 }}>
              {["VST3", "·", "Apple Silicon", "·", "ARM64"].map((t, i) => (
                <span key={i} style={{
                  fontSize: 7, color: T.txtC,
                  letterSpacing: t === "·" ? 0 : 2.2,
                  textTransform: "uppercase",
                  fontFamily: "'DM Mono', monospace",
                  opacity: 0.5,
                }}>{t}</span>
              ))}
            </div>

          </div>
        </div>
      </div>
    </div>
  );
}
