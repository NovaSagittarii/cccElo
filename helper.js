// add timestamps
[...document.querySelectorAll("a:has(sup)")].forEach(x => x.innerText += " -- "+new Date(x.innerText.replace("UTC", " UTC")).getTime());

// get leaderboard
(function(){
    const weights = "100 200 300 400 500 600 700 800 900 ".trim().split(" ").map(x => +x);
    const PENALTY = 5; // in minutes
    const results = [];
    [...document.querySelectorAll('tr[participantid]')].forEach(tr => {
        const username = tr.querySelector('a').innerText.trim();
        let penalty = 0;
        let score = 0;
        const result = [...tr.querySelectorAll('td[problemid]')].map((td, i) => {
            if(td.querySelector('span.cell-accepted')){
                const [incorrect, time] = td.innerText.split('\n');
                const [hh, mm] = time.split(":").map(x => +x);
                let wrong = parseInt(incorrect.replace('+','')) || 0;
                penalty += 60 * (hh*60+ mm + (PENALTY * wrong));
                score += weights[i];
                if(!(i in weights)) throw "couldnt read score weight";
            }
        });
        results.push([username, score, penalty]);
    });
    console.log(results.map(x => x.join(' ')).join('\n'));
})();