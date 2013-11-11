<?
/******
 *  reference: https://code.google.com/p/cool-php-captcha/
 *  jQuery is needed.
 ******/

class Captcha {
    private $codeLength;
    private $codeString;
    private $fontArray;
    private $width;
    private $height;

    private $image;
    private $backColor;
    private $textColor;
    private $noiseColor;

    public function __construct($codeLength = 7, $width = 300, $height = 50) {
        if(!isset($_SESSION)) session_start();
        $this->codeLength = $codeLength;
        $this->width = $width;
        $this->height = $height;
    }

    public function createCaptchaImage($fontLocation = NULL) {
        $this->codeGenerate();

        if(empty($fontLocation)) $fontLocation = dirname(__FILE__)."/ttf/";
        $this->setFontArray($fontLocation);

        $this->image = imagecreate($this->width, $this->height);

        $this->setColors();

        $this->addNoises();

        $this->addCodeString();

        $this->setCaptcha();
        return;
    }

    public function checkCaptcha() {
        if(!isset($_SESSION)) session_start();
        if(!isset($_POST['CAPTCHACODE']) || empty($_SESSION['CAPTCHACODE'])) return false;
        if(strtolower($_POST['CAPTCHACODE']) != strtolower($_SESSION['CAPTCHACODE'])) return false;
        unset($_SESSION['CAPTCHACODE']);
        return true;
    }

    public function captchaImage() {
        $matchSF = preg_split("/\\//", dirname($_SERVER['SCRIPT_FILENAME']));
        $matchF = preg_split("/\\//", dirname(__FILE__));
        $numSF = count($matchSF);
        $numF = count($matchF);
        $offset = $numSF;
        $sf = 0;
        $f = 0;
        while($sf != $numSF && $f != $numF) {
            if($matchSF[$sf] == $matchF[$f]) {
                ++$sf;
                ++$f;
            }
            else break;
        }
        if($sf <= $numSF) $offset = $numSF - $sf;

        $src = preg_replace("/\\/([^\\/]*)$/", "/", $_SERVER['SCRIPT_NAME']);
        for($i = 0; $i < $offset; ++$i) $src .= "../";
        for(; $f < $numF; ++$f) $src .= $matchF[$f]."/";
        $src = "http://".$_SERVER['HTTP_HOST'].$src;

        $captchasrc = "captcha.php";
        $reloadsrc = "reload.jpg";
        $scriptsrc = "captcha.js";
        echo "<img src='".$src.$captchasrc."' id='CAPTCHAIMAGE' alt='captcha' />\n";
        echo "<script src='".$src.$scriptsrc."'></script>\n";
        echo "<a href='javascript: CaptchaReload(\"CAPTCHAIMAGE\", \"".$src.$captchasrc."\");'><img src='".$src.$reloadsrc."' alt='reload' height='".(($this->height)/2)."' /></a>\n";
        return;
    }

    public function captchaInput() {
        echo "<input id='CAPTCHACODE' name='CAPTCHACODE' type='text' />\n";
        return;
    }

    private function codeGenerate() {
        $characterSet = "abcdefghijklmnopqrstuvwxyz1234567890";
        $this->codeString = "";
        $setSize = strlen($characterSet)-1;
        for($i = 0; $i < $this->codeLength; ++$i) {
            $this->codeString .= substr($characterSet, mt_rand(0, $setSize), 1);
        }
        return;
    }

    private function setFontArray($fontLocation) {
        $fontArray = array();

        if(file_exists($fontLocation)) {
            if(is_dir($fontLocation)) {
                $files = scandir($fontLocation);
                foreach($files as &$file) {
                    if(is_file($fontLocation.$file) && preg_match("/\.ttf$/i", $file)) {
                        $this->fontArray[] = realpath($fontLocation.$file);
                    }
                }
            }
            else if(is_file($fontLocation) && preg_match("/\.ttf$/i", $fontLocation)) {
                $this->fontArray[] = realpath($fontLocation);
            }
        }
        if(count($this->fontArray) == 0) {
            echo "Cannot find any fonts (TrueType Font file).\n";
        }
        return;
    }

    private function setColors() {
        $colorSet = array(
                /*** array elements (R, G, B) ***/
                array("0xff", "0xff", "0xff"), 
                array("0x77", "0xdd", "0xff"), 
                array("0xff", "0xa4", "0x88"), 
                array("0x00", "0x88", "0x66"), 
                array("0x64", "0x78", "0xb4"), 
                array("0xaa", "0x00", "0x00"), 
                array("0xcc", "0x66", "0x00"), 
                array("0x88", "0x88", "0x88"), 
                array("0xbb", "0xff", "0x99"), 
                array("0x14", "0x28", "0x64")
                );
        $colorSetSize = count($colorSet)-1;

        $backColorNum = mt_rand(0, $colorSetSize);

        do {
            $textColorNum = mt_rand(0, $colorSetSize);
        } while($textColorNum == $backColorNum);

        do {
            $noiseColorNum = mt_rand(0, $colorSetSize);
        } while($noiseColorNum == $backColorNum || $noiseColorNum == $textColorNum);

        $this->backColor = imagecolorallocate($this->image, $colorSet[$backColorNum][0], $colorSet[$backColorNum][1], $colorSet[$backColorNum][2]);
        imagecolorallocate($this->image, $colorSet[$backColorNum][0], $colorSet[$backColorNum][1], $colorSet[$backColorNum][2]);

        $this->textColor = imagecolorallocate($this->image, $colorSet[$textColorNum][0], $colorSet[$textColorNum][1], $colorSet[$textColorNum][2]);
        $this->noiseColor = imagecolorallocate($this->image, $colorSet[$noiseColorNum][0], $colorSet[$noiseColorNum][1], $colorSet[$noiseColorNum][2]);
        return;
    }

    private function addNoises() {

        /*** add noise dot ***/
        $dotNum = $this->width * $this->height / 10;
        for($i = 0; $i < $dotNum; $i++) {
            imagefilledellipse($this->image, mt_rand(0, $this->width), mt_rand(0, $this->height), 1, 1, $this->noiseColor);
        }

        /*** add noise line ***/
        $lineNum = $this->width * $this->height / 400;
        for($i = 0; $i < $lineNum; $i++) {
            imageline($this->image, mt_rand(0, $this->width), mt_rand(0, $this->height), mt_rand(0, $this->width), mt_rand(0, $this->height), $this->noiseColor);
        }

        /*** add noise curve ***/
        $curveNum = $this->width * $this->height / 400;
        for($i = 0; $i < $curveNum; $i++) {
            imagefilledarc($this->image, mt_rand(0, $this->width), mt_rand(0, $this->height), mt_rand(0, $this->width), mt_rand(0, $this->height), mt_rand(0, 360), mt_rand(0, 360), $this->noiseColor, IMG_ARC_NOFILL);
        }
        return;
    }

    private function addCodeString() {
        $fontSize = $this->width / $this->codeLength*0.85;
        $font = $this->fontArray[mt_rand(0, count($this->fontArray)-1)];
        $textbox = imagettfbbox($fontSize, 0, $font, $this->codeString);
        $x = ($this->width - $textbox[4]) / 2;
        $y = ($this->height - $textbox[5]) / 2;
        imagettftext($this->image, $fontSize, 0, $x, $y, $this->textColor, $font , $this->codeString);
        return;
    }

    private function setCaptcha() {
        header("Content-Type: image/png");
        imagepng($this->image);
        imagedestroy($this->image);

        if(!isset($_SESSION)) session_start();
        $_SESSION['CAPTCHACODE'] = $this->codeString;
        return;
    }
}

$CAPTCHA = new Captcha();
?>
