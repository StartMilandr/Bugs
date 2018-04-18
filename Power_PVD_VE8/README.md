# Power_PVD_VE8

Пример демонстрирует неправильность выставления уровня ULIMIT и вариант его обхода.

Выставление уровня PWR->ULIMIT в большее значение выставляется неправильно - не соответствует установленному напряжению. Предлагается "разгонять" выставление уровня предварительной записью большего значения.

    void PVD_SetLevel(uint32_t pvdLevel, uint32_t delayTicks)
    {
      #ifdef USE_PVD_HACK  
        if (pvdLevel < 0x1F)
          PWR->ULIMIT = pvdLevel + 1;
      #endif
      PWR->ULIMIT = pvdLevel;
    
      if (delayTicks > 0)  
      Delay(delayTicks);  
    }

Подробнее описание примера здесь - https://startmilandr.ru/doku.php/prog:bug:pvd_testve8
