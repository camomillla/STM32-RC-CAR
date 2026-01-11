function client_gui
singletonKey = 'matlab_client_gui_singleton';

if isappdata(0, singletonKey)
    existingFig = getappdata(0, singletonKey);
    if ~isempty(existingFig) && isvalid(existingFig)
        try
            existingFig.Visible = 'on';
            existingFig.WindowState = 'normal';
        catch
        end
        uialert(existingFig, "Aplikacja jest już uruchomiona.", "Informacja");
        return;
    else
        rmappdata(0, singletonKey);
    end
end

bg = [0.12 0.12 0.12];
border = [0.55 0.55 0.55];
txt = [0.92 0.92 0.92];

activeBg = [0.20 0.20 0.20];
activeBorder = [0.95 0.95 0.95];

stopDelay = 0.05;
curlTimeoutSec = 0.2;

fig = uifigure( ...
    'Name','Klient MATLAB', ...
    'Position',[200 200 760 440], ...
    'Resize','off', ...
    'Color',bg);

setappdata(0, singletonKey, fig);

app = struct();
app.baseUrl = "";
app.connected = false;

app.keys = struct('up',false,'down',false,'left',false,'right',false);
app.lastMotor = "1";

app.stopTimer = timer( ...
    'ExecutionMode','singleShot', ...
    'StartDelay',stopDelay, ...
    'TimerFcn',@(~,~)sendStopIfStillIdle());

if isprop(fig,'WindowKeyPressFcn')
    fig.WindowKeyPressFcn = @onKeyPress;
    fig.WindowKeyReleaseFcn = @onKeyRelease;
else
    fig.KeyPressFcn = @onKeyPress;
    fig.KeyReleaseFcn = @onKeyRelease;
end
fig.CloseRequestFcn = @onClose;

ipFrame = uipanel(fig, ...
    'Units','pixels', ...
    'Position',[55 330 410 52], ...
    'BackgroundColor',bg, ...
    'BorderType','line', ...
    'HighlightColor',border);

app.ip = uieditfield(ipFrame,'text', ...
    'Position',[8 8 394 36], ...
    'FontName','Segoe UI', ...
    'FontSize',14, ...
    'FontColor',txt, ...
    'BackgroundColor',bg, ...
    'HorizontalAlignment','center');

try
    app.ip.Placeholder = "ADRES IP";
catch
    app.ip.Value = "ADRES IP";
end

try, app.ip.KeyPressFcn = @onKeyPress; end
try, app.ip.KeyReleaseFcn = @onKeyRelease; end

app.connect = makeBtn(fig,[520 330 140 52],"POŁĄCZ",@onConnect,14,"Segoe UI");

app.engine = makeBtn(fig,[120 205 200 62],"ENGINE",@onEngine,16,"Segoe UI");
app.lights = makeBtn(fig,[120 150 200 50],"LIGHTS",@onLights,16,"Segoe UI");
app.horn   = makeBtn(fig,[120 95  200 50],"HORN",  @onHorn,  16,"Segoe UI");

arrowW = 86;
arrowH = 56;
gap = 0;

dpadX = 400;
dpadY = 90;

x0 = dpadX;
x1 = dpadX + (arrowW + gap);
x2 = dpadX + 2*(arrowW + gap);

y0 = dpadY;
y1 = dpadY + (arrowH + gap);
y2 = dpadY + 2*(arrowH + gap);

app.dl   = makeBtn(fig,[x0 y0 arrowW arrowH],"↙",@(~,~)sendMotor("7"),18,"Segoe UI Symbol");
app.down = makeBtn(fig,[x1 y0 arrowW arrowH],"v", @(~,~)sendMotor("6"),18,"Segoe UI");
app.dr   = makeBtn(fig,[x2 y0 arrowW arrowH],"↘",@(~,~)sendMotor("5"),18,"Segoe UI Symbol");

app.left = makeBtn(fig,[x0 y1 arrowW arrowH],"<",   @(~,~)sendMotor("8"),18,"Segoe UI");
app.stop = makeBtn(fig,[x1 y1 arrowW arrowH],"STOP",@(~,~)sendMotor("1"),16,"Segoe UI");
app.right= makeBtn(fig,[x2 y1 arrowW arrowH],">",   @(~,~)sendMotor("4"),18,"Segoe UI");

app.ul   = makeBtn(fig,[x0 y2 arrowW arrowH],"↖",@(~,~)sendMotor("9"),18,"Segoe UI Symbol");
app.up   = makeBtn(fig,[x1 y2 arrowW arrowH],"^", @(~,~)sendMotor("2"),18,"Segoe UI");
app.ur   = makeBtn(fig,[x2 y2 arrowW arrowH],"↗",@(~,~)sendMotor("3"),18,"Segoe UI Symbol");

setappdata(fig,'app',app);
updateHighlightsByMotor("1");

    function onConnect(~,~)
        app = getappdata(fig,'app');

        ip = strtrim(string(app.ip.Value));
        if ip == "" || upper(ip) == "ADRES IP"
            uialert(fig,"Wpisz adres IP.","Brak IP");
            return;
        end

        app.baseUrl = normalizeBaseUrl(ip);
        setappdata(fig,'app',app);

        ok = sendRequest("/init");
        app = getappdata(fig,'app');
        app.connected = ok;
        setappdata(fig,'app',app);

        if ok
            disp("Połączono (HTTP): " + app.baseUrl);
        else
            disp("Nie udało się połączyć (HTTP): " + app.baseUrl);
        end
    end

    function onEngine(~,~)
        sendRequest("/engine");
    end

    function onLights(~,~)
        sendRequest("/lights");
    end

    function onHorn(~,~)
        sendRequest("/horn");
    end

    function onKeyPress(~, e)
        app = getappdata(fig,'app');
        k = normalizeKey(e);
        changed = false;

        switch k
            case "up",    if ~app.keys.up,    app.keys.up = true;    changed = true; end
            case "down",  if ~app.keys.down,  app.keys.down = true;  changed = true; end
            case "left",  if ~app.keys.left,  app.keys.left = true;  changed = true; end
            case "right", if ~app.keys.right, app.keys.right = true; changed = true; end
            otherwise, return
        end

        setappdata(fig,'app',app);

        if changed
            cancelStopTimer();
            tickMovement();
        end
    end

    function onKeyRelease(~, e)
        app = getappdata(fig,'app');
        k = normalizeKey(e);
        changed = false;

        switch k
            case "up",    if app.keys.up,    app.keys.up = false;    changed = true; end
            case "down",  if app.keys.down,  app.keys.down = false;  changed = true; end
            case "left",  if app.keys.left,  app.keys.left = false;  changed = true; end
            case "right", if app.keys.right, app.keys.right = false; changed = true; end
            otherwise, return
        end

        setappdata(fig,'app',app);

        if changed
            tickMovement();
        end
    end

    function cancelStopTimer()
        app = getappdata(fig,'app');
        if isvalid(app.stopTimer) && strcmp(app.stopTimer.Running,"on")
            stop(app.stopTimer);
        end
    end

    function scheduleStopIfNeeded()
        app = getappdata(fig,'app');
        if app.lastMotor ~= "1" && strcmp(app.stopTimer.Running,"off")
            start(app.stopTimer);
        end
    end

    function sendStopIfStillIdle()
        if ~isvalid(fig)
            return
        end
        app = getappdata(fig,'app');
        if ~anyKeyDown(app.keys) && app.lastMotor ~= "1"
            sendMotor("1");
        end
    end

    function tickMovement()
        if ~isvalid(fig)
            return
        end
        app = getappdata(fig,'app');

        motor = computeMotor(app.keys);
        updateHighlightsByMotor(motor);

        if motor == "1"
            scheduleStopIfNeeded();
            return
        end

        cancelStopTimer();

        if motor ~= app.lastMotor
            sendMotor(motor);
        end
    end

    function motor = computeMotor(keys)
        up = keys.up && ~keys.down;
        down = keys.down && ~keys.up;
        left = keys.left && ~keys.right;
        right = keys.right && ~keys.left;

        if up && right,   motor = "3"; return; end
        if up && left,    motor = "9"; return; end
        if down && right, motor = "5"; return; end
        if down && left,  motor = "7"; return; end

        if up,    motor = "2"; return; end
        if down,  motor = "6"; return; end
        if right, motor = "4"; return; end
        if left,  motor = "8"; return; end

        motor = "1";
    end

    function updateHighlightsByMotor(motor)
        app = getappdata(fig,'app');

        setActive(app.up, false);
        setActive(app.down, false);
        setActive(app.left, false);
        setActive(app.right, false);
        setActive(app.ul, false);
        setActive(app.ur, false);
        setActive(app.dl, false);
        setActive(app.dr, false);
        setActive(app.stop, false);

        switch string(motor)
            case "2", setActive(app.up, true);
            case "3", setActive(app.ur, true);
            case "4", setActive(app.right, true);
            case "5", setActive(app.dr, true);
            case "6", setActive(app.down, true);
            case "7", setActive(app.dl, true);
            case "8", setActive(app.left, true);
            case "9", setActive(app.ul, true);
            otherwise, setActive(app.stop, true);
        end
    end

    function setActive(btnObj, isOn)
        if isOn
            btnObj.frame.HighlightColor = activeBorder;
            btnObj.frame.BackgroundColor = activeBg;
            btnObj.btn.BackgroundColor = activeBg;
        else
            btnObj.frame.HighlightColor = border;
            btnObj.frame.BackgroundColor = bg;
            btnObj.btn.BackgroundColor = bg;
        end
    end

    function ok = sendMotor(motor)
        app = getappdata(fig,'app');
        ok = sendRequest("/motor/" + string(motor));
        app = getappdata(fig,'app');
        app.lastMotor = string(motor);
        setappdata(fig,'app',app);
    end

    function ok = sendRequest(path)
        ok = false;

        app = getappdata(fig,'app');
        if app.baseUrl == ""
            disp(">> (brak IP) " + string(path));
            return
        end

        url = app.baseUrl + string(path);

        cmd = sprintf('curl -X GET --max-time %.3f --connect-timeout %.3f -sS "%s"', ...
            curlTimeoutSec, curlTimeoutSec, char(url));

        disp(">> " + string(url));

        try
            [status, cmdout] = system(cmd);
        catch ME
            disp("<< Błąd system(): " + string(ME.message));
            return
        end

        if isempty(cmdout)
            cmdout = "";
        end

        if status == 0
            ok = true;
            disp("<< " + string(cmdout));
        else
            disp("<< (curl status " + string(status) + ") " + string(cmdout));
        end
    end

    function base = normalizeBaseUrl(ip)
        s = strtrim(string(ip));
        if startsWith(lower(s),"http://") || startsWith(lower(s),"https://")
            base = s;
        else
            base = "http://" + s;
        end
        while endsWith(base,"/")
            base = extractBefore(base, strlength(base));
        end
    end

    function onClose(~,~)
        try
            app = getappdata(fig,'app');
            if isfield(app,'stopTimer') && isvalid(app.stopTimer)
                if strcmp(app.stopTimer.Running,"on")
                    stop(app.stopTimer);
                end
                delete(app.stopTimer);
            end
        catch
        end
        try
            if isappdata(0, singletonKey)
                f = getappdata(0, singletonKey);
                if isequal(f, fig)
                    rmappdata(0, singletonKey);
                end
            end
        catch
        end
        delete(fig);
    end

    function k = normalizeKey(e)
        k = "";

        key = "";
        try
            if isstruct(e)
                if isfield(e,'Key')
                    key = e.Key;
                end
            else
                key = e.Key;
            end
        catch
            key = "";
        end

        key = lower(string(key));

        switch key
            case {"uparrow","up"}
                k = "up";
            case {"downarrow","down"}
                k = "down";
            case {"leftarrow","left"}
                k = "left";
            case {"rightarrow","right"}
                k = "right";
        end
    end

    function tf = anyKeyDown(keys)
        tf = keys.up || keys.down || keys.left || keys.right;
    end

    function obj = makeBtn(parent,pos,text,cb,fontSize,fontName)
        frame = uipanel(parent, ...
            'Units','pixels', ...
            'Position',pos, ...
            'BackgroundColor',bg, ...
            'BorderType','line', ...
            'HighlightColor',border);

        btn = uibutton(frame,'push', ...
            'Text',text, ...
            'Position',[0 0 pos(3) pos(4)], ...
            'FontName',fontName, ...
            'FontSize',fontSize, ...
            'FontColor',txt, ...
            'BackgroundColor',bg);

        if ~isempty(cb)
            btn.ButtonPushedFcn = cb;
        end

        obj = struct('frame',frame,'btn',btn);
    end
end
