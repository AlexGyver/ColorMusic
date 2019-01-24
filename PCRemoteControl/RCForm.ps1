<#
 Форма для управления цветомузыкой с ПК
 В скетче Arduino укажите define REMOTE_TYPE 4
#>

Add-Type -AssemblyName System.Windows.Forms
[System.Windows.Forms.Application]::EnableVisualStyles()

<#
 # Найдено ли Arduino-устройство
 # @var bool
 #>
$devFound = $false

<#
 # Поток к Arduino устройству
 # @var Serial
 #>
$arduino = $null

# Ищем устаройства CH340 - так определяется Arduino в диспетчере задач
# Если у вашего устройства другой чип, поменяйте на следующей строке CH340 на другое значение
$query = Get-WmiObject -Query "Select Caption from Win32_PnPEntity WHERE Name like '%CH340%'"
foreach ($port in $query){
    try{
        # Ищем COM порты
        if($port['Caption'] -match 'COM\d+'){
            $com = $Matches[0] 
            $baud = 9600
            $arduino = new-Object System.IO.Ports.SerialPort $com,$baud,None,8,one

            Write-host "Connect to " $arduino.PortName " port..."

            $arduino.ReadTimeout = 5000;
            $arduino.WriteTimeout = 5000;
            $arduino.open()

            # Отправляем приветствие
            Write-host "handshake ..."
            $arduino.WriteLine("handshake")
            Write-host "Read line ..."
            start-sleep -m 50
            
            $hello = $arduino.ReadLine()
            Write-host "Answer:" $hello
            
            # Если вернуло нашу команду - считываем следующую строку
            if($hello -match "handshake"){
                Write-host "Read next"
                start-sleep -m 50
                $hello = $arduino.ReadLine()
                Write-host "Next answer:" $hello
            }

            # Если нашли название ColorMusic, то это наше Arduino
            if($hello -match "ColorMusic"){
                Write-host "Device found"
                $devFound = $true
                break
            }

            # $arduino.close()
        }
       
    } catch {
        Write-host "Error"
        $_
    }


    if($arduino.isOpen){
        $arduino.close()
    }
}        
        
<#  Форма с ошибкой  #>
$ErrorForm                       = New-Object system.Windows.Forms.Form
$ErrorForm.ClientSize            = '463,120'
$ErrorForm.text                  = "Error"
$ErrorForm.TopMost               = $false
$ErrorForm.FormBorderStyle       = 'Fixed3D'
$ErrorForm.MaximizeBox           = $false
$ErrorForm.MinimizeBox           = $false

$ELabel1                          = New-Object system.Windows.Forms.Label
$ELabel1.text                     = " "
$ELabel1.BackColor                = "#ff0000"
$ELabel1.AutoSize                 = $false
$ELabel1.width                    = 39
$ELabel1.height                   = 41
$ELabel1.location                 = New-Object System.Drawing.Point(24,33)
$ELabel1.Font                     = 'Microsoft Sans Serif,10,style=Bold'
$ELabel1.ForeColor                = "#ffffff"

$ELabel2                          = New-Object system.Windows.Forms.Label
$ELabel2.text                     = "X"
$ELabel2.AutoSize                 = $true
$ELabel2.width                    = 25
$ELabel2.height                   = 10
$ELabel2.location                 = New-Object System.Drawing.Point(30,37)
$ELabel2.Font                     = 'Microsoft Sans Serif,20,style=Bold'
$ELabel2.ForeColor                = "#ffffff"
$ELabel2.BackColor                = "#ff0000"

$LabelError                      = New-Object system.Windows.Forms.Label
$LabelError.text                 = "Ошибка: не найдено устройство для удалённого управления!"
$LabelError.AutoSize             = $false
$LabelError.width                = 360
$LabelError.height               = 42
$LabelError.Anchor               = 'top,right,left'
$LabelError.location             = New-Object System.Drawing.Point(78,34)
$LabelError.Font                 = 'Microsoft Sans Serif,10'

$EButton1                         = New-Object system.Windows.Forms.Button
$EButton1.text                    = "OK"
$EButton1.width                   = 60
$EButton1.height                  = 30
$EButton1.location                = New-Object System.Drawing.Point(393,85)
$EButton1.Font                    = 'Microsoft Sans Serif,10'

$EButton1.Add_Click({  
    $ErrorForm.close()
})

$ErrorForm.controls.AddRange(@($ELabel2, $ELabel1, $LabelError, $EButton1))

<# 
    Форма для управления цветомузыкой
    https://poshgui.com/editor/5c47451d742c2245c890cd9c
#>
$Form                            = New-Object system.Windows.Forms.Form
$Form.ClientSize                 = '605,370'
$Form.text                       = "<Arduino> ColorMusic"
$Form.TopMost                    = $false
$Form.FormBorderStyle            = 'Fixed3D'
$Form.MaximizeBox                = $false

$btn_power                       = New-Object system.Windows.Forms.Button
$btn_power.BackColor             = "#ca2929"
$btn_power.text                  = "Power"
$btn_power.width                 = 60
$btn_power.height                = 30
$btn_power.location              = New-Object System.Drawing.Point(7,10)
$btn_power.Font                  = 'Microsoft Sans Serif,10'
$btn_power.ForeColor             = "#ffffff"

$btn_calibrate                   = New-Object system.Windows.Forms.Button
$btn_calibrate.text              = "Калибровка"
$btn_calibrate.width             = 100
$btn_calibrate.height            = 30
$btn_calibrate.location          = New-Object System.Drawing.Point(80,10)
$btn_calibrate.Font              = 'Microsoft Sans Serif,10'

$btn_volume_gradient             = New-Object system.Windows.Forms.Button
$btn_volume_gradient.text        = "Градиент"
$btn_volume_gradient.width       = 100
$btn_volume_gradient.height      = 30
$btn_volume_gradient.location    = New-Object System.Drawing.Point(10,22)
$btn_volume_gradient.Font        = 'Microsoft Sans Serif,10'

$btn_volume_rainbow              = New-Object system.Windows.Forms.Button
$btn_volume_rainbow.text         = "Радуга"
$btn_volume_rainbow.width        = 100
$btn_volume_rainbow.height       = 30
$btn_volume_rainbow.location     = New-Object System.Drawing.Point(125,22)
$btn_volume_rainbow.Font         = 'Microsoft Sans Serif,10'

$Label2                          = New-Object system.Windows.Forms.Label
$Label2.text                     = "Яркость"
$Label2.AutoSize                 = $false
$Label2.width                    = 112
$Label2.height                   = 18
$Label2.Anchor                   = 'top,right,left'
$Label2.location                 = New-Object System.Drawing.Point(82,25)
$Label2.Font                     = 'Microsoft Sans Serif,10'

$btn_bright_inc                  = New-Object system.Windows.Forms.Button
$btn_bright_inc.text             = "+"
$btn_bright_inc.width            = 25
$btn_bright_inc.height           = 25
$btn_bright_inc.location         = New-Object System.Drawing.Point(43,19)
$btn_bright_inc.Font             = 'Microsoft Sans Serif,10'

$btn_bright_dec                  = New-Object system.Windows.Forms.Button
$btn_bright_dec.text             = "-"
$btn_bright_dec.width            = 25
$btn_bright_dec.height           = 25
$btn_bright_dec.location         = New-Object System.Drawing.Point(10,19)
$btn_bright_dec.Font             = 'Microsoft Sans Serif,10'

$btn_color_5                     = New-Object system.Windows.Forms.Button
$btn_color_5.text                = "5 полос"
$btn_color_5.width               = 100
$btn_color_5.height              = 30
$btn_color_5.location            = New-Object System.Drawing.Point(10,22)
$btn_color_5.Font                = 'Microsoft Sans Serif,10'

$btn_color_3                     = New-Object system.Windows.Forms.Button
$btn_color_3.text                = "3 полосы"
$btn_color_3.width               = 100
$btn_color_3.height              = 30
$btn_color_3.location            = New-Object System.Drawing.Point(120,22)
$btn_color_3.Font                = 'Microsoft Sans Serif,10'

$btn_color_1                     = New-Object system.Windows.Forms.Button
$btn_color_1.text                = "1 полоса"
$btn_color_1.width               = 100
$btn_color_1.height              = 30
$btn_color_1.location            = New-Object System.Drawing.Point(230,22)
$btn_color_1.Font                = 'Microsoft Sans Serif,10'

$btn_color_run                   = New-Object system.Windows.Forms.Button
$btn_color_run.text              = "Бегущие частоты"
$btn_color_run.width             = 150
$btn_color_run.height            = 30
$btn_color_run.location          = New-Object System.Drawing.Point(10,64)
$btn_color_run.Font              = 'Microsoft Sans Serif,10'

$btn_back_dec                    = New-Object system.Windows.Forms.Button
$btn_back_dec.text               = "-"
$btn_back_dec.width              = 25
$btn_back_dec.height             = 25
$btn_back_dec.location           = New-Object System.Drawing.Point(10,54)
$btn_back_dec.Font               = 'Microsoft Sans Serif,10'

$btn_back_inc                    = New-Object system.Windows.Forms.Button
$btn_back_inc.text               = "+"
$btn_back_inc.width              = 25
$btn_back_inc.height             = 25
$btn_back_inc.location           = New-Object System.Drawing.Point(43,54)
$btn_back_inc.Font               = 'Microsoft Sans Serif,10'

$btn_analyze                     = New-Object system.Windows.Forms.Button
$btn_analyze.text                = "Анализатор"
$btn_analyze.width               = 150
$btn_analyze.height              = 30
$btn_analyze.location            = New-Object System.Drawing.Point(180,64)
$btn_analyze.Font                = 'Microsoft Sans Serif,10'

$btn_left                        = New-Object system.Windows.Forms.Button
$btn_left.text                   = "-"
$btn_left.width                  = 25
$btn_left.height                 = 25
$btn_left.location               = New-Object System.Drawing.Point(10,101)
$btn_left.Font                   = 'Microsoft Sans Serif,10'

$btn_right                       = New-Object system.Windows.Forms.Button
$btn_right.text                  = "+"
$btn_right.width                 = 25
$btn_right.height                = 25
$btn_right.location              = New-Object System.Drawing.Point(43,101)
$btn_right.Font                  = 'Microsoft Sans Serif,10'

$btn_down                        = New-Object system.Windows.Forms.Button
$btn_down.text                   = "-"
$btn_down.width                  = 25
$btn_down.height                 = 25
$btn_down.location               = New-Object System.Drawing.Point(10,136)
$btn_down.Font                   = 'Microsoft Sans Serif,10'

$btn_up                          = New-Object system.Windows.Forms.Button
$btn_up.text                     = "+"
$btn_up.width                    = 25
$btn_up.height                   = 25
$btn_up.location                 = New-Object System.Drawing.Point(43,136)
$btn_up.Font                     = 'Microsoft Sans Serif,10'

$btn_const                       = New-Object system.Windows.Forms.Button
$btn_const.text                  = "Подсветка"
$btn_const.width                 = 100
$btn_const.height                = 30
$btn_const.location              = New-Object System.Drawing.Point(10,22)
$btn_const.Font                  = 'Microsoft Sans Serif,10'

$Groupbox1                       = New-Object system.Windows.Forms.Groupbox
$Groupbox1.height                = 65
$Groupbox1.width                 = 342
$Groupbox1.text                  = "Шкала громкости"
$Groupbox1.location              = New-Object System.Drawing.Point(10,60)

$Groupbox2                       = New-Object system.Windows.Forms.Groupbox
$Groupbox2.height                = 301
$Groupbox2.width                 = 235
$Groupbox2.Anchor                = 'top,right,bottom,left'
$Groupbox2.text                  = "Настройки"
$Groupbox2.location              = New-Object System.Drawing.Point(363,60)

$Label1                          = New-Object system.Windows.Forms.Label
$Label1.text                     = "Яркость фона"
$Label1.AutoSize                 = $false
$Label1.width                    = 112
$Label1.height                   = 18
$Label1.Anchor                   = 'top,right,left'
$Label1.location                 = New-Object System.Drawing.Point(82,58)
$Label1.Font                     = 'Microsoft Sans Serif,10'

$label_lr                        = New-Object system.Windows.Forms.Label
$label_lr.text                   = "LR"
$label_lr.AutoSize               = $false
$label_lr.width                  = 140
$label_lr.height                 = 18
$label_lr.Anchor                 = 'top,right,left'
$label_lr.location               = New-Object System.Drawing.Point(82,105)
$label_lr.Font                   = 'Microsoft Sans Serif,10'

$Groupbox3                       = New-Object system.Windows.Forms.Groupbox
$Groupbox3.height                = 104
$Groupbox3.width                 = 342
$Groupbox3.text                  = "Цветомузыка"
$Groupbox3.location              = New-Object System.Drawing.Point(10,143)

$label_ud                        = New-Object system.Windows.Forms.Label
$label_ud.text                   = "UD"
$label_ud.AutoSize               = $false
$label_ud.width                  = 140
$label_ud.height                 = 18
$label_ud.Anchor                 = 'top,right,left'
$label_ud.location               = New-Object System.Drawing.Point(82,141)
$label_ud.Font                   = 'Microsoft Sans Serif,10'

$Groupbox4                       = New-Object system.Windows.Forms.Groupbox
$Groupbox4.height                = 100
$Groupbox4.width                 = 342
$Groupbox4.text                  = "Постоянная подсветка"
$Groupbox4.location              = New-Object System.Drawing.Point(10,261)

$btn_const_grad                  = New-Object system.Windows.Forms.Button
$btn_const_grad.text             = "Градиент"
$btn_const_grad.width            = 100
$btn_const_grad.height           = 30
$btn_const_grad.location         = New-Object System.Drawing.Point(120,22)
$btn_const_grad.Font             = 'Microsoft Sans Serif,10'

$btn_const_rainbow               = New-Object system.Windows.Forms.Button
$btn_const_rainbow.text          = "Радуга"
$btn_const_rainbow.width         = 100
$btn_const_rainbow.height        = 30
$btn_const_rainbow.location      = New-Object System.Drawing.Point(230,22)
$btn_const_rainbow.Font          = 'Microsoft Sans Serif,10'

$btn_const_wildfire              = New-Object system.Windows.Forms.Button
$btn_const_wildfire.text         = "Огонь"
$btn_const_wildfire.width        = 150
$btn_const_wildfire.height       = 30
$btn_const_wildfire.location     = New-Object System.Drawing.Point(10,60)
$btn_const_wildfire.Font         = 'Microsoft Sans Serif,10'

$btn_const_strobo                = New-Object system.Windows.Forms.Button
$btn_const_strobo.text           = "Стробоскоп"
$btn_const_strobo.width          = 150
$btn_const_strobo.height         = 30
$btn_const_strobo.location       = New-Object System.Drawing.Point(180,60)
$btn_const_strobo.Font           = 'Microsoft Sans Serif,10'

$Form.controls.AddRange(@($btn_power,$btn_calibrate,$Groupbox1,$Groupbox2,$Groupbox3,$Groupbox4))
$Groupbox1.controls.AddRange(@($btn_volume_gradient,$btn_volume_rainbow))
$Groupbox2.controls.AddRange(@($Label2,$btn_bright_inc,$btn_bright_dec,$btn_back_dec,$btn_back_inc,$btn_left,$btn_right,$btn_down,$btn_up,$Label1,$label_lr,$label_ud))
$Groupbox3.controls.AddRange(@($btn_color_5,$btn_color_3,$btn_color_1,$btn_color_run,$btn_analyze))
$Groupbox4.controls.AddRange(@($btn_const,$btn_const_grad,$btn_const_rainbow,$btn_const_wildfire,$btn_const_strobo))

$label_lr.text = " "
$btn_right.Enabled = $false
$btn_left.Enabled = $false

$label_ud.text = " "
$btn_up.Enabled = $false
$btn_down.Enabled = $false

### Callbacks

$btn_power.Add_Click({ 
    $arduino.writeLine("power")
})

$btn_calibrate.Add_Click({  
    $arduino.writeLine("calibrate")
})

$btn_volume_gradient.Add_Click({
    $arduino.writeLine("mode 1")

    $label_lr.text = "Плавность анимации"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = " "
    $btn_up.Enabled = $false
    $btn_down.Enabled = $false
})

$btn_volume_rainbow.Add_Click({  
    $arduino.writeLine("mode 2")

    $label_lr.text = "Плавность анимации"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Скорость радуги"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_color_1.Add_Click({ 
    $arduino.writeLine("mode 5")

    $label_lr.text = "Плавность анимации"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Чувствительность"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_color_3.Add_Click({
    $arduino.writeLine("mode 4")

    $label_lr.text = "Плавность анимации"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Чувствительность"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_color_5.Add_Click({  
    $arduino.writeLine("mode 3")

    $label_lr.text = "Плавность анимации"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Чувствительность"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_const_strobo.Add_Click({  
    $arduino.writeLine("mode 6")

    $label_lr.text = "Плавность вспышек"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Частота вспышек"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_const.Add_Click({  
    $arduino.writeLine("mode 7")
    $arduino.writeLine("lightmode 0")

    $label_lr.text = "Цвет"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Насыщенность"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_const_grad.Add_Click({  
    $arduino.writeLine("mode 7")
    $arduino.writeLine("lightmode 1")

    $label_lr.text = "Скорость"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Насыщенность"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_const_rainbow.Add_Click({  
    $arduino.writeLine("mode 7")
    $arduino.writeLine("lightmode 2")

    $label_lr.text = "Скорость"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Шаг радуги"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_const_wildfire.Add_Click({  
    $arduino.writeLine("mode 7")
    $arduino.writeLine("lightmode 3")

    $label_lr.text = "Искры"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Высота огня"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_color_run.Add_Click({  
    $arduino.writeLine("mode 8")

    $label_lr.text = "Скорость"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Чувствительность"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_analyze.Add_Click({  
    $arduino.writeLine("mode 9")

    $label_lr.text = "Шаг цвета"
    $btn_right.Enabled = $true
    $btn_left.Enabled = $true

    $label_ud.text = "Цвет"
    $btn_up.Enabled = $true
    $btn_down.Enabled = $true
})

$btn_bright_dec.Add_Click({  
    $arduino.writeLine("bright-")
})

$btn_bright_inc.Add_Click({  
    $arduino.writeLine("bright+")
})

$btn_back_inc.Add_Click({
    $arduino.writeLine("backlight+")
})

$btn_back_dec.Add_Click({  
    $arduino.writeLine("backlight-")
})

$btn_up.Add_Click({  
    $arduino.writeLine("up")
})

$btn_down.Add_Click({  
    $arduino.writeLine("down")
})

$btn_right.Add_Click({  
    $arduino.writeLine("right")
})

$btn_left.Add_Click({  
    $arduino.writeLine("left")
})

### GUI End ###

if ($devFound) {
    $Form.text = $Form.text + ' (' + $arduino.PortName + ')'
    $Form.showDialog()
} else {
    $ErrorForm.showDialog()
}

$arduino.close()