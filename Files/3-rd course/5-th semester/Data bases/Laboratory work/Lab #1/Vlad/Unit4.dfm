object Form4: TForm4
  Left = 0
  Top = 0
  Caption = 'Form4'
  ClientHeight = 552
  ClientWidth = 985
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object PageControl1: TPageControl
    Left = 0
    Top = 0
    Width = 985
    Height = 552
    ActivePage = TabSheet1
    Align = alClient
    TabOrder = 0
    object TabSheet1: TTabSheet
      Caption = 'TabSheet1'
      object Label1: TLabel
        Left = 0
        Top = 486
        Width = 977
        Height = 15
        Align = alBottom
        Caption = #1054#1087#1080#1089#1072#1085#1080#1077
        ExplicitWidth = 55
      end
      object DBText1: TDBText
        AlignWithMargins = True
        Left = 3
        Top = 504
        Width = 971
        Height = 15
        Align = alBottom
        AutoSize = True
        DataField = 'gcDescription'
        DataSource = DataModule2.DS_T_goods_catalog
        WordWrap = True
        ExplicitWidth = 54
      end
      object DBGrid1: TDBGrid
        Left = 0
        Top = 0
        Width = 977
        Height = 233
        Align = alTop
        DataSource = DataModule2.DS_Q_v_goods_catalog
        Options = [dgTitles, dgIndicator, dgColumnResize, dgColLines, dgRowLines, dgTabs, dgRowSelect, dgConfirmDelete, dgCancelOnExit, dgTitleClick, dgTitleHotTrack]
        ReadOnly = True
        TabOrder = 0
        TitleFont.Charset = DEFAULT_CHARSET
        TitleFont.Color = clWindowText
        TitleFont.Height = -12
        TitleFont.Name = 'Segoe UI'
        TitleFont.Style = []
        Columns = <
          item
            Expanded = False
            FieldName = 'tgName'
            Title.Caption = #1058#1080#1087' '#1090#1086#1074#1072#1088#1072
            Visible = True
          end
          item
            Expanded = False
            FieldName = 'gcName'
            Title.Caption = #1053#1072#1079#1074#1072#1085#1080#1077
            Width = 562
            Visible = True
          end
          item
            Expanded = False
            FieldName = 'gcCost'
            Title.Caption = #1062#1077#1085#1072
            Width = 73
            Visible = True
          end>
      end
      object InsertEditDeletePanel: TPanel
        Left = 0
        Top = 436
        Width = 977
        Height = 50
        Align = alBottom
        TabOrder = 1
        object InsertButton: TButton
          Left = 408
          Top = 6
          Width = 152
          Height = 38
          Caption = #1042#1089#1090#1072#1074#1080#1090#1100
          TabOrder = 0
          OnClick = InsertButtonClick
        end
        object EditButton: TButton
          Left = 232
          Top = 6
          Width = 151
          Height = 38
          Caption = #1048#1079#1084#1077#1085#1080#1090#1100
          TabOrder = 1
          OnClick = EditButtonClick
        end
        object DeleteButton: TButton
          Left = 587
          Top = 6
          Width = 150
          Height = 38
          Caption = #1059#1076#1072#1083#1080#1090#1100
          TabOrder = 2
          OnClick = DeleteButtonClick
        end
      end
      object PanelToInputValues: TPanel
        Left = 0
        Top = 332
        Width = 977
        Height = 104
        Align = alBottom
        TabOrder = 2
        object Label2: TLabel
          Left = 24
          Top = 14
          Width = 60
          Height = 15
          Caption = #1058#1080#1087' '#1090#1086#1074#1072#1088#1072
        end
        object Label3: TLabel
          Left = 185
          Top = 14
          Width = 52
          Height = 15
          Caption = #1053#1072#1079#1074#1072#1085#1080#1077
        end
        object Label4: TLabel
          Left = 789
          Top = 14
          Width = 28
          Height = 15
          Caption = #1062#1077#1085#1072
        end
        object Label5: TLabel
          Left = 24
          Top = 64
          Width = 58
          Height = 15
          Caption = #1054#1087#1080#1089#1072#1085#1080#1077':'
        end
        object DBLookupComboBox1: TDBLookupComboBox
          Left = 24
          Top = 35
          Width = 145
          Height = 23
          DataField = 'IDtg'
          DataSource = DataModule2.DS_T_goods_catalog
          KeyField = 'IDtg'
          ListField = 'tgName'
          ListSource = DataModule2.DS_T_type_goods
          TabOrder = 0
        end
        object DBEdit1: TDBEdit
          Left = 185
          Top = 35
          Width = 568
          Height = 23
          DataField = 'gcName'
          DataSource = DataModule2.DS_T_goods_catalog
          TabOrder = 1
        end
        object DBEdit2: TDBEdit
          Left = 789
          Top = 35
          Width = 171
          Height = 23
          DataField = 'gcCost'
          DataSource = DataModule2.DS_T_goods_catalog
          TabOrder = 2
        end
        object DBEdit3: TDBEdit
          Left = 185
          Top = 64
          Width = 568
          Height = 23
          DataField = 'gcDescription'
          DataSource = DataModule2.DS_T_goods_catalog
          TabOrder = 3
        end
        object PostButton: TButton
          Left = 789
          Top = 66
          Width = 75
          Height = 25
          Caption = #1047#1072#1087#1086#1084#1085#1080#1090#1100
          TabOrder = 4
          OnClick = PostButtonClick
        end
        object CancelButton: TButton
          Left = 885
          Top = 66
          Width = 75
          Height = 25
          Caption = #1054#1090#1084#1077#1085#1080#1090#1100
          TabOrder = 5
          OnClick = CancelButtonClick
        end
      end
    end
    object TabSheet2: TTabSheet
      Caption = 'TabSheet2'
      ImageIndex = 1
      object DBGrid2: TDBGrid
        Left = 0
        Top = 0
        Width = 977
        Height = 522
        Align = alClient
        DataSource = DataModule2.DS_T_type_goods
        TabOrder = 0
        TitleFont.Charset = DEFAULT_CHARSET
        TitleFont.Color = clWindowText
        TitleFont.Height = -12
        TitleFont.Name = 'Segoe UI'
        TitleFont.Style = []
        Columns = <
          item
            Expanded = False
            FieldName = 'IDtg'
            Title.Caption = 'ID '#1082#1072#1090#1077#1075#1086#1088#1080#1080
            Visible = True
          end
          item
            Expanded = False
            FieldName = 'tgName'
            Title.Caption = #1053#1072#1079#1074#1072#1085#1080#1077' '#1082#1072#1090#1077#1075#1086#1088#1080#1080
            Visible = True
          end>
      end
    end
  end
end
